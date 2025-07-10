#include "pinctrl.h"
#include "gpio.h"
#include "soc_osal.h"
#include "app_init.h"
#include <stdint.h>
#include <stdio.h>
#include "dht11.h"
#include "uart.h"

static void dht11_udelay(uint32_t us)
{
    osal_udelay(us);
}

static void set_gpio_direction(gpio_direction_t dir)
{
    if (uapi_gpio_set_dir(DHT11_PIN, dir) != ERRCODE_SUCC) {
        osal_printk("Set GPIO dir failed!\n");
    }
}

static void dht11_start_signal(void)
{
    osal_printk("Sending start signal...\n");
    set_gpio_direction(GPIO_DIRECTION_OUTPUT);
    uapi_gpio_set_val(DHT11_PIN, GPIO_LEVEL_HIGH);
    dht11_udelay(1000);      // 1ms准备

    uapi_gpio_set_val(DHT11_PIN, GPIO_LEVEL_LOW);
    dht11_udelay(18000);     // 18ms拉低

    uapi_gpio_set_val(DHT11_PIN, GPIO_LEVEL_HIGH);
    dht11_udelay(30);        // 30us拉高等待响应

    set_gpio_direction(GPIO_DIRECTION_INPUT);
    dht11_udelay(100);       // 100us释放总线
}

static int dht11_wait_response(void)
{
    uint32_t timeout = 150;
    while (uapi_gpio_get_val(DHT11_PIN)) {
        if (--timeout == 0) {
            osal_printk("Timeout waiting for LOW response\n");
            return -1;
        }
        dht11_udelay(1);
    }

    timeout = 150;
    while (!uapi_gpio_get_val(DHT11_PIN)) {
        if (--timeout == 0) {
            osal_printk("Timeout waiting for HIGH response\n");
            return -1;
        }
        dht11_udelay(1);
    }

    return 0;
}

static uint8_t dht11_read_bit(void)
{
    uint32_t timeout = 100; // 100μs超时
    
    // 等待起始低电平
    while (uapi_gpio_get_val(DHT11_PIN)) {
        if (--timeout == 0) return 0xFF; // 超时返回特殊值
        dht11_udelay(1);
    }
    
    // 等待低电平结束
    timeout = 100;
    while (!uapi_gpio_get_val(DHT11_PIN)) {
        if (--timeout == 0) return 0xFF; // 超时返回特殊值
        dht11_udelay(1);
    }
    
    // 测量高电平时间
    uint32_t duration = 0;
    timeout = 100;
    while (uapi_gpio_get_val(DHT11_PIN)) {
        dht11_udelay(1);
        duration++;
        if (--timeout == 0) break;
    }

    return (duration > 20) ? 1 : 0;  // 阈值20us
}

static uint8_t dht11_read_byte(void)
{
    uint8_t value = 0;
    for (int i = 0; i < 8; i++) {
        value <<= 1;
        uint8_t bit = dht11_read_bit();
        if (bit == 0xFF) { // 超时保护
            return 0xFF;
        }
        value |= bit;
    }
    return value;
}

static int dht11_read(dht11_data_t *data)
{
    uint8_t bytes[5] = {0};

    for (int attempt = 0; attempt < 3; attempt++) {
        dht11_start_signal();

        if (dht11_wait_response() == 0) {
            int error = 0;
            for (int i = 0; i < 5; i++) {
                bytes[i] = dht11_read_byte();
                // 如果读取到无效字节，提前结束
                if (bytes[i] == 0xFF) {
                    osal_printk("Byte read error at %d\n", i);
                    error = 1;
                    break;
                }
            }
            
            if (error) continue;

            // 计算校验和
            uint8_t checksum = bytes[0] + bytes[1] + bytes[2] + bytes[3];
            osal_printk("Raw: %02X %02X %02X %02X %02X\n",
                        bytes[0], bytes[1], bytes[2], bytes[3], bytes[4]);

            if (checksum == bytes[4]) {
                data->humidity = (float)bytes[0];
                data->temperature = (float)bytes[2];
                data->checksum = (float)bytes[4];
                return 0;
            } else {
                osal_printk("Checksum error: calc=%02X, recv=%02X\n", checksum, bytes[4]);
            }
        } else {
            osal_printk("No response attempt %d\n", attempt);
        }

        osal_msleep(30);
    }

    return -1;
}

void dht11_task(const char *arg)
{
    unused(arg);
    dht11_data_t dht_data;
    
    // 堆栈保护 - 使用数组占用空间但不引发警告
    volatile uint8_t stack_guard[STACK_GUARD_SIZE];
    (void)stack_guard; // 避免未使用变量警告

    // 打印任务信息
    osal_printk("DHT11 task started, stack size: %d bytes\n", DHT11_TASK_STACK_SIZE);

    uapi_pin_set_mode(DHT11_PIN, HAL_PIO_FUNC_GPIO);
    uapi_gpio_set_dir(DHT11_PIN, GPIO_DIRECTION_OUTPUT);
    uapi_gpio_set_val(DHT11_PIN, GPIO_LEVEL_HIGH);

    // 设置上拉
    errcode_t ret = uapi_pin_set_pull(DHT11_PIN, PIN_PULL_TYPE_STRONG_UP);
    if (ret != ERRCODE_SUCC) {
        osal_printk("Strong pull-up failed, trying normal pull-up\n");
        ret = uapi_pin_set_pull(DHT11_PIN, PIN_PULL_TYPE_UP);
        if (ret != ERRCODE_SUCC) {
            osal_printk("Pull-up config failed!\n");
        }
    }

    osal_msleep(1500);  // 等待传感器上电稳定

    while (1) {
        if (dht11_read(&dht_data) == 0) {
            // 构建数据帧: 帧头(0xAA) | 传感器类型 | 温度 | 湿度 | 校验和 | 帧尾(0x55)
            uint8_t frame[6] = {
                0xAA,                           // 帧头
                SENSOR_DHT11,                   // 传感器类型
                (uint8_t)dht_data.temperature,   // 温度整数部分
                (uint8_t)dht_data.humidity,      // 湿度整数部分
                0,                              // 校验和占位
                0x55                            // 帧尾
            };
            
            // 计算校验和（帧头+类型+温度+湿度）
            frame[4] = (frame[0] + frame[1] + frame[2] + frame[3]) & 0xFF;
            
            // 发送数据
            uapi_uart_write(CONFIG_SLE_UART_BUS, frame,sizeof(frame), 0);

            osal_printk("DHT11: Temp=%.1f°C, Humid=%.1f%%\n", 
                       dht_data.temperature, dht_data.humidity);
        } else {
            osal_printk("DHT11 read failed\n");
        }
        osal_msleep(DHT11_READ_INTERVAL_MS);
    }
}