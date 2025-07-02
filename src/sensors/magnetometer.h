#ifndef MAGNETOMETER_H
#define MAGNETOMETER_H

#include <stdint.h>
#include "accelerometer.h"

#define MAGNETOMETER_I2C_ADDR 0x1E
#define CFG_REG_A_M 0x60// Magnetometer control register
#define MAGNETOMETER_INIT 0x8C // Continuous mode + temp compensation (0x80)
#define OUTX_L_REG_M 0x68 // Output X low register
#define DATA_REG_M (OUTX_L_REG_M | 0x80) // Auto-increment bit
#define MT_LSB 0.15f // Magnetometer LSB in µT (0.15 µT/LSB for 16-bit resolution)
#define MAG_DECLINATION_DEG 12.78f
// Hard Iron Offset Registers

static inline int16_t float_to_q8_7(float x) {
    if (x > 127.992f) x = 127.992f;
    else if (x < -128.0f) x = -128.0f;
    return (int16_t)(x * 128.0f);
}

static inline float q8_7_to_float(int16_t x) {
    return (float)x / 128.0f;
}

#define OFFSET_X_REG_L_M 0x45 // Offset X low register



static uint8_t magnetometer_config[] = { CFG_REG_A_M, MAGNETOMETER_INIT };

typedef struct {
    int16_t heading, pitch, roll;  // Angles in degrees
} MagnetometerAngleData;

typedef struct {
    int16_t x, y, z;  // Magnetic field data in microteslas (µT)
    MagnetometerAngleData angle; // Angles in degrees
} MagnetometerData;

void magnetometer_init(void);
void magnetometer_read(MagnetometerData *data, const AccelerometerData *accel_data);
MagnetometerAngleData magnetometer_angle(const MagnetometerData *data, const AccelerometerData *accel_data);
void calibrate_magnetometer(void);
void reset_calibration(void);
void calibrate_raw_data(float mx, float my, float mz, float *cal_x, float *cal_y, float *cal_z);
#endif
