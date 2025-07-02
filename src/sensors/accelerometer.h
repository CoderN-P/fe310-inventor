#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include <stdint.h>
#define SCALE_FACTOR_8G 0.0039f // Scale factor for 8G hi-res mode (G/LSB)
#define ACCELEROMETER_I2C_ADDR 0x19
#define CTRL_REG1_A 0x20
#define CTRL_REG4_A 0x23 // Control register 4 for accelerometer
#define ACCELEROMETER_INIT 0x77 // Normal mode, 400Hz, X, Y, Z enabled
#define ACCELEROMETER_INIT_4 0xA8 // 8G scale, BDU enabled, high-res mode
#define OUT_X_L_A 0x28
#define DATA_REG_A (OUT_X_L_A | 0x80) // Auto-increment bit
#define Q4_11_SCALE 2048 // 2^11


static uint8_t accelerometer_config[] = { CTRL_REG1_A, ACCELEROMETER_INIT };
static uint8_t accelerometer_config_2[] = { CTRL_REG4_A, ACCELEROMETER_INIT_4 };

typedef int16_t q4_11_t; // 4.11 fixed-point format

#define float_to_q4_11(x) ((q4_11_t)((x) * Q4_11_SCALE)) // Convert float to 4.11 fixed-point format
#define q4_11_to_float(x) ((float)(x) / Q4_11_SCALE) // Convert 4.11 fixed-point format to float
#define q4_11_int(x)   ((x) >> 11)
#define q4_11_frac(x)  ({                   \
    int32_t __tmp = (x);                   \
    if (__tmp < 0) __tmp = -__tmp;         \
    (__tmp & 0x7FF) * 1000 / 2048;         \
})

typedef struct {
    q4_11_t x, y, z;
} AccelerometerData;

void accelerometer_init(void);
void accelerometer_read(AccelerometerData *data);

#endif