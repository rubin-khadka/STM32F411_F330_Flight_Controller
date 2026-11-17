/*
 * mpu6050.h
 *
 *  Created on: Nov 13, 2026
 *      Author: Rubin Khadka
 */

#ifndef MPU6050_MPU6050_H_
#define MPU6050_MPU6050_H_

#include <stdint.h>
#include "stm32f4xx_hal.h"

/* 7-bit I2C address */
#define MPU6050_ADDR_7BIT           (0x68U)

/* Register map */
#define MPU6050_REG_SMPLRT_DIV      (0x19U)
#define MPU6050_REG_CONFIG          (0x1AU)
#define MPU6050_REG_GYRO_CONFIG     (0x1BU)
#define MPU6050_REG_ACCEL_CONFIG    (0x1CU)
#define MPU6050_REG_INT_PIN_CFG     (0x37U)
#define MPU6050_REG_INT_ENABLE      (0x38U)
#define MPU6050_REG_INT_STATUS      (0x3AU)
#define MPU6050_REG_ACCEL_XOUT_H    (0x3BU)
#define MPU6050_REG_PWR_MGMT_1      (0x6BU)
#define MPU6050_REG_WHO_AM_I        (0x75U)

#define MPU6050_BURST_LEN           (14U)

/* WHO_AM_I expected values */
#define MPU_ID_MPU6050              (0x68U)

/* INT_STATUS bit masks */
#define MPU6050_INT_DATA_RDY        (0x01U)

/* Configuration constants */
#define MPU6050_I2C_TIMEOUT_MS      (5U)
#define MPU6050_DLPF_CFG            (0x03U)   /* 44Hz accel / 42Hz gyro */
#define MPU6050_SMPLRT_DIV_VAL      (0x01U)   /* 1kHz / 2 = 500Hz */
#define MPU6050_GYRO_FS_SEL         (0x08U)   /* ±500 °/s */
#define MPU6050_ACCEL_AFS_SEL       (0x08U)   /* ±4 g */
#define MPU6050_INT_PIN_CFG_VAL     (0x00U)   /* push-pull, active-high, 50µs pulse */
#define MPU6050_INT_ENABLE_VAL      (MPU6050_INT_DATA_RDY)

/* Scale factors matching the FS_SEL/AFS_SEL above */
#define MPU6050_ACCEL_LSB_PER_G     (8192.0f)   /* ±4g */
#define MPU6050_GYRO_LSB_PER_DPS    (65.5f)     /* ±500°/s */
#define MPU6050_TEMP_LSB_PER_C      (340.0f)
#define MPU6050_TEMP_OFFSET_C       (36.53f)

/* Calibration */
#define MPU6050_CAL_SAMPLES         (2000U)
#define MPU6050_CAL_DELAY_MS        (1U)

typedef enum {
    MPU6050_OK = 0,
    MPU6050_ERR_INVALID_ARG,
    MPU6050_ERR_NOT_FOUND,
    MPU6050_ERR_READ,
    MPU6050_ERR_WRITE,
    MPU6050_ERR_NOT_CALIBRATED
} MPU6050_Status_t;

typedef struct {
    float ax_g;
    float ay_g;
    float az_g;
    float gx_dps;
    float gy_dps;
    float gz_dps;
    float temp_c;
} MPU6050_Data_t;

MPU6050_Status_t MPU6050_Init(I2C_HandleTypeDef *hi2c);
MPU6050_Status_t MPU6050_Read(MPU6050_Data_t *out);
MPU6050_Status_t MPU6050_CalibrateGyro(void);
void             MPU6050_GetGyroBias(float *bx, float *by, float *bz);
uint32_t         MPU6050_GetErrorCount(void);
MPU6050_Status_t MPU6050_IsDataReady(uint8_t *ready);

#endif /* MPU6050_MPU6050_H_ */
