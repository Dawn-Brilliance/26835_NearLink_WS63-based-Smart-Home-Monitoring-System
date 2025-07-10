/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * Description: SLE UART Sample Source. \n
 *
 * History: \n
 * 2023-07-17, Create file. \n
 */

#include "common_def.h"
#include "soc_osal.h"
#include "app_init.h"
#include "pinctrl.h"
#include "uart.h"
#include "i2c.h"
#include "osal_debug.h"
#include <unistd.h>
#include "sle_low_latency.h"

#if defined(CONFIG_SAMPLE_SUPPORT_SENSOR_SLE_SERVER)
#include "securec.h"
#include "./sensor_sle_server/sle_uart_server.h"
#include "./sensor_sle_server/sle_uart_server_adv.h"
#include "sle_device_discovery.h"
#include "sle_errcode.h"
#include "./dht11/dht11.h"
#elif defined(CONFIG_SAMPLE_SUPPORT_SENSOR_SLE_CLIENT)
#include "sle_connection_manager.h"
#include "sle_ssap_client.h"
#include "./sensor_sle_client/sle_uart_client.h"
#include "./max30102/max30102.h"
#include "./adxl345/adxl345.h"
#endif  /* CONFIG_SAMPLE_SUPPORT_SLE_UART_CLIENT */

#define SLE_UART_TASK_PRIO                  28
#define SLE_UART_TASK_DURATION_MS           50
#define SLE_UART_BAUDRATE                   115200
#define SLE_UART_TRANSFER_SIZE              1024
#define SLE_UART_TASK_STACK_SIZE            0x1200
#define I2C_TASK_STACK_SIZE                 0x2000
#define I2C_TASK_PRIO                       10

// I2C引脚定义
#define CONFIG_I2C_SCL_MASTER_PIN 15
#define CONFIG_I2C_SDA_MASTER_PIN 16
#define CONFIG_I2C_MASTER_PIN_MODE 2

#if defined(CONFIG_SAMPLE_SUPPORT_SENSOR_SLE_SERVER)
#define SLE_UART_SERVER_DELAY_COUNT         5
#define SLE_UART_TASK_STACK_SIZE            0x1200
#define SLE_ADV_HANDLE_DEFAULT              1
#define SLE_UART_SERVER_MSG_QUEUE_LEN       5
#define SLE_UART_SERVER_MSG_QUEUE_MAX_SIZE  32
#define SLE_UART_SERVER_QUEUE_DELAY         0xFFFFFFFF
#define SLE_UART_SERVER_BUFF_MAX_SIZE       800


#ifdef CONFIG_SAMPLE_SUPPORT_PERFORMANCE_TYPE
#define SLE_UART_SERVER_SEND_BUFF_MAX_LEN   250
#else
#define SLE_UART_SERVER_SEND_BUFF_MAX_LEN   40
#endif
unsigned long g_sle_uart_server_msgqueue_id;
#define SLE_UART_SERVER_LOG                 "[sle uart server]"
#define UART_TX_QUEUE_SIZE 20

static uint8_t g_app_uart_rx_buff[SLE_UART_TRANSFER_SIZE] = { 0 };
static uart_buffer_config_t g_app_uart_buffer_config = {
    .rx_buffer = g_app_uart_rx_buff,
    .rx_buffer_size = SLE_UART_TRANSFER_SIZE
};


static void uart_init_pin(void)
{
    if (CONFIG_SLE_UART_BUS == 0) {
        uapi_pin_set_mode(CONFIG_UART_TXD_PIN, PIN_MODE_1);
        uapi_pin_set_mode(CONFIG_UART_RXD_PIN, PIN_MODE_1);       
    }else if (CONFIG_SLE_UART_BUS == 1) {
        uapi_pin_set_mode(CONFIG_UART_TXD_PIN, PIN_MODE_1);
        uapi_pin_set_mode(CONFIG_UART_RXD_PIN, PIN_MODE_1);       
    }
}

static errcode_t uart_init_config(void)
{
    uart_attr_t attr = {
        .baud_rate = SLE_UART_BAUDRATE,
        .data_bits = UART_DATA_BIT_8,
        .stop_bits = UART_STOP_BIT_1,
        .parity = UART_PARITY_NONE
    };

    uart_pin_config_t pin_config = {
        .tx_pin = CONFIG_UART_TXD_PIN,
        .rx_pin = CONFIG_UART_RXD_PIN,
        .cts_pin = PIN_NONE,
        .rts_pin = PIN_NONE
    };
    uapi_uart_deinit(CONFIG_SLE_UART_BUS);

    // 检查初始化结果
     errcode_t ret = uapi_uart_init(CONFIG_SLE_UART_BUS, &pin_config, &attr, 
                                  NULL, &g_app_uart_buffer_config);
    if (ret != ERRCODE_SUCC) {
        osal_printk("%s UART init failed: %x\r\n", SLE_UART_SERVER_LOG, ret);
        return ret;
    }
    
    return ERRCODE_SUCC;
}


static void ssaps_server_read_request_cbk(uint8_t server_id, uint16_t conn_id, ssaps_req_read_cb_t *read_cb_para,
    errcode_t status)
{
    osal_printk("%s ssaps read request cbk callback server_id:%x, conn_id:%x, handle:%x, status:%x\r\n",
        SLE_UART_SERVER_LOG, server_id, conn_id, read_cb_para->handle, status);
}

static void ssaps_server_write_request_cbk(uint8_t server_id, uint16_t conn_id, 
                                          ssaps_req_write_cb_t *write_cb_para,
                                          errcode_t status)
{
    (void)server_id;
    (void)conn_id;
    (void)write_cb_para;
    (void)status;

    if ((write_cb_para->length > 0) && write_cb_para->value) 
    {
        // 直接转发原始数据到串口
        uapi_uart_write(CONFIG_SLE_UART_BUS, write_cb_para->value, write_cb_para->length, 0);
    }  
}


static void sle_uart_server_read_int_handler(const void *buffer, uint16_t length, bool error)
{
    (void)buffer;  // 避免未使用参数的警告
    (void)length;  // 避免未使用参数的警告
    unused(error);
    if (sle_uart_client_is_connected()) {
    } else {
        osal_printk("%s sle client is not connected! \r\n", SLE_UART_SERVER_LOG);
    }
}

static void sle_uart_server_create_msgqueue(void)
{
    if (osal_msg_queue_create("sle_uart_server_msgqueue", SLE_UART_SERVER_MSG_QUEUE_LEN, \
        (unsigned long *)&g_sle_uart_server_msgqueue_id, 0, SLE_UART_SERVER_MSG_QUEUE_MAX_SIZE) != OSAL_SUCCESS) {
        osal_printk("^%s sle_uart_server_create_msgqueue message queue create failed!\n", SLE_UART_SERVER_LOG);
    }
}

static void sle_uart_server_delete_msgqueue(void)
{
    osal_msg_queue_delete(g_sle_uart_server_msgqueue_id);
}

static void sle_uart_server_write_msgqueue(uint8_t *buffer_addr, uint16_t buffer_size)
{
    osal_msg_queue_write_copy(g_sle_uart_server_msgqueue_id, (void *)buffer_addr, \
                              (uint32_t)buffer_size, 0);
}

static int32_t sle_uart_server_receive_msgqueue(uint8_t *buffer_addr, uint32_t *buffer_size)
{
    return osal_msg_queue_read_copy(g_sle_uart_server_msgqueue_id, (void *)buffer_addr, \
                                    buffer_size, SLE_UART_SERVER_QUEUE_DELAY);
}
static void sle_uart_server_rx_buf_init(uint8_t *buffer_addr, uint32_t *buffer_size)
{
    *buffer_size = SLE_UART_SERVER_MSG_QUEUE_MAX_SIZE;
    (void)memset_s(buffer_addr, *buffer_size, 0, *buffer_size);
}


static void *sle_uart_server_task(const char *arg)
{
    unused(arg);
    uint8_t rx_buf[SLE_UART_SERVER_MSG_QUEUE_MAX_SIZE] = {0};
    uint32_t rx_length = SLE_UART_SERVER_MSG_QUEUE_MAX_SIZE;
    uint8_t sle_connect_state[] = "sle_dis_connect";

    /* UART pinmux. */
    uart_init_pin();

    /* UART init config. */
    if (uart_init_config() != ERRCODE_SUCC) {
        osal_printk("%s FATAL: UART init failed! Task halted.\r\n", SLE_UART_SERVER_LOG);
        while (1) osal_msleep(1000); // 阻塞任务
    }
    uapi_pin_init();
    
     // 创建DHT11任务
    
    osal_task *dht11_task_handle = osal_kthread_create(
        (osal_kthread_handler)dht11_task,
        NULL,
        "DHT11Task",
        DHT11_TASK_STACK_SIZE
    );
    if (dht11_task_handle != NULL) {
        osal_kthread_set_priority(dht11_task_handle, DHT11_TASK_PRIO);
        osal_printk("%s DHT11 task created\n", SLE_UART_SERVER_LOG);
    } else {
        osal_printk("%s Failed to create DHT11 task!\n", SLE_UART_SERVER_LOG);
    }

    // 延迟启动TX任务（确保UART就绪）
    osal_msleep(50);
    
    sle_uart_server_create_msgqueue();
    sle_uart_server_register_msg(sle_uart_server_write_msgqueue);
    sle_uart_server_init(ssaps_server_read_request_cbk, ssaps_server_write_request_cbk);
    
    uapi_uart_unregister_rx_callback(CONFIG_SLE_UART_BUS);

    errcode_t ret = uapi_uart_register_rx_callback(CONFIG_SLE_UART_BUS,
                                                   UART_RX_CONDITION_FULL_OR_SUFFICIENT_DATA_OR_IDLE,
                                                   1, sle_uart_server_read_int_handler);
    if (ret != ERRCODE_SUCC) {
        osal_printk("%s Register uart callback fail.[%x]\r\n", SLE_UART_SERVER_LOG, ret);
        return NULL;
    }
    
    
    while (1) {
        sle_uart_server_rx_buf_init(rx_buf, &rx_length);
        sle_uart_server_receive_msgqueue(rx_buf, &rx_length);
        if (strncmp((const char *)rx_buf, (const char *)sle_connect_state, sizeof(sle_connect_state)) == 0) {
            ret = sle_start_announce(SLE_ADV_HANDLE_DEFAULT);
            if (ret != ERRCODE_SLE_SUCCESS) {
                osal_printk("%s sle_connect_state_changed_cbk,sle_start_announce fail :%02x\r\n",
                    SLE_UART_SERVER_LOG, ret);
            }
        }
        osal_msleep(SLE_UART_TASK_DURATION_MS);
    }

    sle_uart_server_delete_msgqueue();
    return NULL;
}

#elif defined(CONFIG_SAMPLE_SUPPORT_SENSOR_SLE_CLIENT)

extern volatile uint16_t g_active_conn_id;

static void app_i2c_init_pin(void)
{
    osal_printk("[I2C] Configuring SCL: Pin %d, Mode %d\n", 
               CONFIG_I2C_SCL_MASTER_PIN, CONFIG_I2C_MASTER_PIN_MODE);
    uapi_pin_set_mode(CONFIG_I2C_SCL_MASTER_PIN, CONFIG_I2C_MASTER_PIN_MODE);
    
    osal_printk("[I2C] Configuring SDA: Pin %d, Mode %d\n", 
               CONFIG_I2C_SDA_MASTER_PIN, CONFIG_I2C_MASTER_PIN_MODE);
    uapi_pin_set_mode(CONFIG_I2C_SDA_MASTER_PIN, CONFIG_I2C_MASTER_PIN_MODE);
}

// 初始化I2C总线
static errcode_t sensors_i2c_init(void)
{
    // 先初始化引脚
    app_i2c_init_pin();
    
    // 初始化I2C总线1
    osal_printk("[I2C] Initializing I2C bus %d...\n", ADXL345_I2C_BUS);
    errcode_t ret = uapi_i2c_master_init(ADXL345_I2C_BUS, ADXL345_BAUDRATE, 0);
    if (ret != ERRCODE_SUCC) {
        osal_printk("[I2C] Init failed: 0x%X\n", ret);
        return ret;
    }
    osal_printk("[I2C] Initialized successfully\n");
    return ERRCODE_SUCC;
}

void sle_uart_notification_cb(uint8_t client_id, uint16_t conn_id, ssapc_handle_value_t *data,
    errcode_t status)
{
    unused(client_id);
    unused(conn_id);
    unused(status);
    osal_printk("Received data: %.*s\n", data->data_len, data->data);
}

void sle_uart_indication_cb(uint8_t client_id, uint16_t conn_id, ssapc_handle_value_t *data,
    errcode_t status)
{
    unused(client_id);
    unused(conn_id);
    unused(status);
    osal_printk("Received data: %.*s\n", data->data_len, data->data);
}

static void sensor_collection_task(void *arg) {
    (void)arg;
    
    // 初始化所有传感器
    if (adxl345_init() != 0) {
        osal_printk("[SENSOR] ADXL345 init failed\n");
    }

    (void)arg;
    #define SAMPLE_INTERVAL 10

    static uint32_t ir_buffer[BUFFER_SIZE] = {0};
    static uint32_t red_buffer[BUFFER_SIZE] = {0};
    int32_t spo2 = 0, heart_rate = 0;
    int8_t spo2_valid = 0, hr_valid = 0;
    int buffer_index = 0;
    //uint32_t fail_count = 0;
   
    osal_printk("[MAX30102] Initializing sensor...\n");

    if (MAX30102_Init() != 0) {
        osal_printk("[SENSOR] MAX30102 init failed\n");
    }
    osal_msleep(100);

   
    int16_t x_raw, y_raw, z_raw;

    int accel_counter=0;
    while (1) {
        if(accel_counter==49) {
            // 读取加速度数据
            adxl345_read_xyz(&x_raw, &y_raw, &z_raw);
            
            // 修复帧结构：确保帧尾是0x55
            uint8_t frame[10] = {
                0xAA,
                0x01,  // ADXL345类型标识
                (uint8_t)(x_raw & 0xFF),
                (uint8_t)((x_raw >> 8) & 0xFF),
                (uint8_t)(y_raw & 0xFF),
                (uint8_t)((y_raw >> 8) & 0xFF),
                (uint8_t)(z_raw & 0xFF),
                (uint8_t)((z_raw >> 8) & 0xFF),
                (0xAA + 0x01 + 
                 (uint8_t)(x_raw & 0xFF) + (uint8_t)((x_raw >> 8) & 0xFF) +
                 (uint8_t)(y_raw & 0xFF) + (uint8_t)((y_raw >> 8) & 0xFF) +
                 (uint8_t)(z_raw & 0xFF) + (uint8_t)((z_raw >> 8) & 0xFF)) & 0xFF, // 校验和
                0x55   // 确保帧尾是0x55
            };

            // 调试输出
            osal_printk("[ADXL] Sending: ");
            for (size_t i = 0; i < sizeof(frame); i++) {
                osal_printk("%02X ", frame[i]);
            }
            osal_printk("\n");
            
            // 发送数据
            ssapc_write_param_t *send_param = get_g_sle_uart_send_param();
            send_param->data_len = sizeof(frame);
            send_param->data = frame;
            errcode_t send_ret = ssapc_write_req(0, get_g_sle_uart_conn_id(), send_param);
            if (send_ret != ERRCODE_SUCC) {
                osal_printk("[SLE] Send failed: 0x%X\n", send_ret);
            }
            accel_counter = 0;
        }
        accel_counter ++;
        // 再读取MAX30102
        uint32_t ir_data, red_data;
        maxim_max30102_read_fifo(&red_data, &ir_data);


        ir_buffer[buffer_index] = ir_data;
        red_buffer[buffer_index] = red_data;
        buffer_index = (buffer_index + 1) % BUFFER_SIZE;
        
        if (buffer_index == 0) {
            maxim_heart_rate_and_oxygen_saturation(
                ir_buffer, BUFFER_SIZE, red_buffer,
                &spo2, &spo2_valid, &heart_rate, &hr_valid
            );


            if (spo2_valid && hr_valid) {
                osal_printk("[SENSOR] Valid Data: HR=%d, SpO2=%d\n", heart_rate, spo2);
                
                // 构建数据包
                uint8_t frame[7] = {
                    0xAA,
                    0x02,  // MAX30102类型标识
                    heart_rate & 0xFF,
                    (heart_rate >> 8) & 0xFF,
                    spo2 & 0xFF,
                    (0xAA + 0x02 + (heart_rate & 0xFF) + ((heart_rate >> 8) & 0xFF) + (spo2 & 0xFF)) & 0xFF, // 校验和
                    0x55
                };
                
                // 调试输出数据包内容
                osal_printk("[SLE] Sending: ");
                for (size_t i = 0; i < sizeof(frame); i++) {
                    osal_printk("%02X ", frame[i]);
                }
                osal_printk("\n");
                // 通过SLE发送
                ssapc_write_param_t *send_param = get_g_sle_uart_send_param();
                if (send_param && get_g_sle_uart_conn_id() != 0xFFFF) {
                    send_param->data_len = sizeof(frame);
                    send_param->data = frame;
                    errcode_t send_ret = ssapc_write_req(0, get_g_sle_uart_conn_id(), send_param);
                    if (send_ret != ERRCODE_SUCC) {
                        osal_printk("[SLE] Send failed: 0x%X\n", send_ret);
                    }
                } else {
                    osal_printk("[SLE] Connection not ready\n");
                }
            } 
        }
        osal_msleep(10);
    }
}

static void *sle_uart_client_task(const char *arg) {
    (void)arg;

     // 初始化I2C总线
    if (sensors_i2c_init() != ERRCODE_SUCC) {
        osal_printk("[SENSOR] FATAL: I2C init failed! Task halted.\n");
        while(1) osal_msleep(1000); // 阻塞任务
    }
    
    // 初始化SLE客户端
    sle_uart_client_init(sle_uart_notification_cb, sle_uart_indication_cb);

    // 等待连接建立 (最多10秒)
    uint32_t timeout = 100;
    while (g_active_conn_id == 0xFFFF && timeout--) {
        osal_printk("[CLIENT] Waiting for connection... Timeout=%lu\n", timeout);
        osal_msleep(100);
    }
    
    if (g_active_conn_id == 0xFFFF) {
        osal_printk("[SENSOR] FATAL: Connection timeout!\n");
        return NULL;
    }

   // 单任务处理所有传感器
    osal_task *sensor_task_handle = osal_kthread_create(
        (osal_kthread_handler)sensor_collection_task, 
        NULL, 
        "SensorTask",
        I2C_TASK_STACK_SIZE
    );
    
    if (sensor_task_handle) {
        osal_kthread_set_priority(sensor_task_handle, I2C_TASK_PRIO);
    }
    
    while (1) { osal_msleep(1000); }
    return NULL;
}
#endif  /* CONFIG_SAMPLE_SUPPORT_SLE_UART_CLIENT */

static void sle_uart_entry(void)
{
    osal_task *task_handle = NULL;
    osal_kthread_lock();
#if defined(CONFIG_SAMPLE_SUPPORT_SENSOR_SLE_SERVER)
    task_handle = osal_kthread_create((osal_kthread_handler)sle_uart_server_task, 0, "SLEUartServerTask",
                                      SLE_UART_TASK_STACK_SIZE);
#elif defined(CONFIG_SAMPLE_SUPPORT_SENSOR_SLE_CLIENT)
    task_handle = osal_kthread_create((osal_kthread_handler)sle_uart_client_task, 0, "SLEUartDongleTask",
                                      SLE_UART_TASK_STACK_SIZE);
#endif /* CONFIG_SAMPLE_SUPPORT_SLE_UART_CLIENT */
    if (task_handle != NULL) {
        osal_kthread_set_priority(task_handle, SLE_UART_TASK_PRIO);
    }
    osal_kthread_unlock();
}

/* Run the sle_uart_entry. */
app_run(sle_uart_entry);