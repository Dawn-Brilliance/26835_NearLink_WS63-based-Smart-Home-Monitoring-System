#include "adxl345.h"
#include "common_def.h"
#include "soc_osal.h"
#include "app_init.h"
#include "pinctrl.h"
#include "uart.h"
#include "i2c.h"
#include "osal_debug.h"
#include <unistd.h>
#include "gpio.h"


errcode_t adxl345_write(uint8_t reg, uint8_t value)
{
    uint8_t buf[2] = {reg, value};
    i2c_data_t data = {
        .send_buf = buf,
        .send_len = 2,
        .receive_buf = NULL,
        .receive_len = 0,
    };
    return uapi_i2c_master_write(ADXL345_I2C_BUS, ADXL345_I2C_ADDR, &data);
}

errcode_t adxl345_read(uint8_t reg, uint8_t *recv_buf, uint32_t len)
{
    i2c_data_t data = {
        .send_buf = &reg,
        .send_len = 1,
        .receive_buf = recv_buf,
        .receive_len = len,
    };
    return uapi_i2c_master_writeread(ADXL345_I2C_BUS, ADXL345_I2C_ADDR, &data);
}

errcode_t adxl345_init(void)
{  
  uint8_t devid = 0;
    errcode_t ret = adxl345_read(ADXL345_REG_DEVID, &devid, 1);
    if (ret != ERRCODE_SUCC) {
        osal_printk("[ADXL345] Read DEVID failed: 0x%X\n", ret);
        return ret;
    } else if (devid != 0xE5) {
        osal_printk("[ADXL345] Invalid ID: 0x%02X (expected 0xE5)\n", devid);
        return ERRCODE_FAIL;
    }

    ret = adxl345_write(ADXL345_REG_DATA_FORMAT, 0x08); // full res, ±2g
    if (ret != ERRCODE_SUCC) {
        osal_printk("[ADXL345] Set DATA_FORMAT failed: 0x%X\n", ret);
        return ret;
    }
    
    ret = adxl345_write(ADXL345_REG_POWER_CTL, 0x08);   // enable measurement
    if (ret != ERRCODE_SUCC) {
        osal_printk("[ADXL345] Set POWER_CTL failed: 0x%X\n", ret);
        return ret;
    }

    return ERRCODE_SUCC;
}

void adxl345_read_xyz(int16_t *x, int16_t *y, int16_t *z)
{
    uint8_t buf[6] = {0};
    if (adxl345_read(ADXL345_REG_DATAX0, buf, 6) != 0) {
        printf("Read XYZ failed\n");
        *x = *y = *z = 0;
        return;
    }
    
    *x = (int16_t)((buf[1] << 8) | buf[0]);
    *y = (int16_t)((buf[3] << 8) | buf[2]);
    *z = (int16_t)((buf[5] << 8) | buf[4]);
}