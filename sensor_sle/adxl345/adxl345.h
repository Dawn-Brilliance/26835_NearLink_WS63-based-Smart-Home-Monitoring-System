#ifndef ADXL345_H
#define ADXL345_H
#include <stdint.h>
#include "i2c.h" // 包含海思I2C驱动
#include "stdbool.h"

// ADXL345 I2C相关定义
#define ADXL345_I2C_BUS         1
#define ADXL345_I2C_ADDR        0x53
#define ADXL345_BAUDRATE        400000


// ADXL345寄存器地址
#define ADXL345_REG_DEVID       0x00
#define ADXL345_REG_POWER_CTL   0x2D
#define ADXL345_REG_DATA_FORMAT 0x31
#define ADXL345_REG_DATAX0      0x32

errcode_t adxl345_write(uint8_t reg, uint8_t value);
errcode_t adxl345_read(uint8_t reg, uint8_t *recv_buf, uint32_t len);
errcode_t adxl345_init(void);
void adxl345_read_xyz(int16_t *x, int16_t *y, int16_t *z);

#endif