/*
 * mpu6050.c
 *
 *  Created on: Nov 13, 2026
 *      Author: Rubin Khadka
 */

#include "mpu6050.h"
#include <string.h>

static I2C_HandleTypeDef *s_hi2c = NULL;
static float    s_bias_x = 0.0f;
static float    s_bias_y = 0.0f;
static float    s_bias_z = 0.0f;
static uint8_t  s_calibrated = 0U;
static uint32_t s_err_count  = 0U;

/* Convert two big-endian bytes to a signed 16-bit integer safely. */
static int16_t mpu_be_to_i16(uint8_t msb, uint8_t lsb)
{
    uint16_t u16 = ((uint16_t)msb << 8U) | (uint16_t)lsb;
    int16_t i16;
    memcpy(&i16, &u16, sizeof(i16));   /* avoid strict-aliasing UB */
    return i16;
}

static MPU6050_Status_t mpu_read_regs(uint8_t reg, uint8_t *buf, uint16_t len)
{
    if (HAL_I2C_Mem_Read(s_hi2c,
                         (uint16_t)(MPU6050_ADDR_7BIT << 1U),
                         (uint16_t)reg,
                         I2C_MEMADD_SIZE_8BIT,
                         buf, len,
                         MPU6050_I2C_TIMEOUT_MS) != HAL_OK) {
        s_err_count++;
        return MPU6050_ERR_READ;
    }
    return MPU6050_OK;
}

static MPU6050_Status_t mpu_write_reg(uint8_t reg, uint8_t value)
{
    if (HAL_I2C_Mem_Write(s_hi2c,
                          (uint16_t)(MPU6050_ADDR_7BIT << 1U),
                          (uint16_t)reg,
                          I2C_MEMADD_SIZE_8BIT,
                          &value, 1U,
                          MPU6050_I2C_TIMEOUT_MS) != HAL_OK) {
        s_err_count++;
        return MPU6050_ERR_WRITE;
    }
    return MPU6050_OK;
}

MPU6050_Status_t MPU6050_Init(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == NULL) return MPU6050_ERR_INVALID_ARG;
    s_hi2c = hi2c;

    /* 1. Verify chip presence */
    uint8_t who = 0U;
    if (mpu_read_regs(MPU6050_REG_WHO_AM_I, &who, 1U) != MPU6050_OK)
        return MPU6050_ERR_NOT_FOUND;
    if (who != MPU_ID_MPU6050)
        return MPU6050_ERR_NOT_FOUND;

    /* 2. Device reset */
    if (mpu_write_reg(MPU6050_REG_PWR_MGMT_1, 0x80U) != MPU6050_OK)
        return MPU6050_ERR_WRITE;
    HAL_Delay(100U);

    /* 3. Wake + PLL X-gyro clock source */
    if (mpu_write_reg(MPU6050_REG_PWR_MGMT_1, 0x01U) != MPU6050_OK)
        return MPU6050_ERR_WRITE;
    HAL_Delay(50U);

    /* 4. Sample rate divider */
    if (mpu_write_reg(MPU6050_REG_SMPLRT_DIV, MPU6050_SMPLRT_DIV_VAL) != MPU6050_OK)
        return MPU6050_ERR_WRITE;

    /* 5. DLPF */
    if (mpu_write_reg(MPU6050_REG_CONFIG, MPU6050_DLPF_CFG) != MPU6050_OK)
        return MPU6050_ERR_WRITE;

    /* 6. Gyro full-scale */
    if (mpu_write_reg(MPU6050_REG_GYRO_CONFIG, MPU6050_GYRO_FS_SEL) != MPU6050_OK)
        return MPU6050_ERR_WRITE;

    /* 7. Accel full-scale */
    if (mpu_write_reg(MPU6050_REG_ACCEL_CONFIG, MPU6050_ACCEL_AFS_SEL) != MPU6050_OK)
        return MPU6050_ERR_WRITE;

    /* 8. INT pin: push-pull, active-high, 50 µs pulse */
    if (mpu_write_reg(MPU6050_REG_INT_PIN_CFG, MPU6050_INT_PIN_CFG_VAL) != MPU6050_OK)
        return MPU6050_ERR_WRITE;

    /* 9. Enable data-ready interrupt on INT pin */
    if (mpu_write_reg(MPU6050_REG_INT_ENABLE, MPU6050_INT_ENABLE_VAL) != MPU6050_OK)
        return MPU6050_ERR_WRITE;

    s_calibrated = 0U;
    s_err_count  = 0U;
    return MPU6050_OK;
}

MPU6050_Status_t MPU6050_Read(MPU6050_Data_t *out)
{
    if (out == NULL || s_hi2c == NULL)
        return MPU6050_ERR_INVALID_ARG;

    uint8_t buf[MPU6050_BURST_LEN];
    if (mpu_read_regs(MPU6050_REG_ACCEL_XOUT_H, buf, MPU6050_BURST_LEN) != MPU6050_OK)
        return MPU6050_ERR_READ;

    int16_t ax = mpu_be_to_i16(buf[0],  buf[1]);
    int16_t ay = mpu_be_to_i16(buf[2],  buf[3]);
    int16_t az = mpu_be_to_i16(buf[4],  buf[5]);
    int16_t t  = mpu_be_to_i16(buf[6],  buf[7]);
    int16_t gx = mpu_be_to_i16(buf[8],  buf[9]);
    int16_t gy = mpu_be_to_i16(buf[10], buf[11]);
    int16_t gz = mpu_be_to_i16(buf[12], buf[13]);

    out->ax_g   = (float)ax / MPU6050_ACCEL_LSB_PER_G;
    out->ay_g   = (float)ay / MPU6050_ACCEL_LSB_PER_G;
    out->az_g   = (float)az / MPU6050_ACCEL_LSB_PER_G;
    out->gx_dps = (float)gx / MPU6050_GYRO_LSB_PER_DPS - s_bias_x;
    out->gy_dps = (float)gy / MPU6050_GYRO_LSB_PER_DPS - s_bias_y;
    out->gz_dps = (float)gz / MPU6050_GYRO_LSB_PER_DPS - s_bias_z;
    out->temp_c = (float)t  / MPU6050_TEMP_LSB_PER_C + MPU6050_TEMP_OFFSET_C;

    return MPU6050_OK;
}

MPU6050_Status_t MPU6050_CalibrateGyro(void)
{
    if (s_hi2c == NULL) return MPU6050_ERR_INVALID_ARG;

    /* Zero biases so Read() returns raw values during calibration */
    s_bias_x = s_bias_y = s_bias_z = 0.0f;

    MPU6050_Data_t d;
    float sx = 0.0f, sy = 0.0f, sz = 0.0f;

    for (uint32_t i = 0U; i < MPU6050_CAL_SAMPLES; i++) {
        if (MPU6050_Read(&d) != MPU6050_OK)
            return MPU6050_ERR_READ;
        sx += d.gx_dps;
        sy += d.gy_dps;
        sz += d.gz_dps;
        HAL_Delay(MPU6050_CAL_DELAY_MS);
    }

    s_bias_x = sx / (float)MPU6050_CAL_SAMPLES;
    s_bias_y = sy / (float)MPU6050_CAL_SAMPLES;
    s_bias_z = sz / (float)MPU6050_CAL_SAMPLES;
    s_calibrated = 1U;
    return MPU6050_OK;
}

void MPU6050_GetGyroBias(float *bx, float *by, float *bz)
{
    if (bx) *bx = s_bias_x;
    if (by) *by = s_bias_y;
    if (bz) *bz = s_bias_z;
}

uint32_t MPU6050_GetErrorCount(void)
{
    return s_err_count;
}

MPU6050_Status_t MPU6050_IsDataReady(uint8_t *ready)
{
    if (ready == NULL || s_hi2c == NULL) return MPU6050_ERR_INVALID_ARG;

    uint8_t status = 0U;
    if (mpu_read_regs(MPU6050_REG_INT_STATUS, &status, 1U) != MPU6050_OK)
        return MPU6050_ERR_READ;

    *ready = (status & MPU6050_INT_DATA_RDY) ? 1U : 0U;
    return MPU6050_OK;
}
