//
// Created by Neel P on 6/16/25.
//

#ifndef LIGHT_H
#define LIGHT_H

// LTR-329 Light Sensor

#include <stdint.h>
#define LIGHT_I2C_ADDR 0x29
#define ALS_CONTROL_REG 0x80
#define ALS_MEAS_RATE_REG 0x85
#define ALS_ACTIVE_MODE 0x01
#define ALS_GAIN_1X 0x00
#define ALS_CONTROL_CMD (ALS_ACTIVE_MODE | ALS_GAIN_1X) // Control command to enable light sensor in active mode with 1x gain
#define ALS_MEAS_RATE_CMD 0x03 // Measurement rate command (100ms integration time) (500ms repeat rate)
#define ALS_DATA_CH0_LOW 0x8A // Visible + IR only (low byte)
#define ALS_DATA_CH0_HIGH 0x8B // Visible + IR only (high byte)
#define ALS_DATA_CH1_LOW 0x88 // IR only (low byte)
#define ALS_DATA_CH1_HIGH 0x89 // IR only (high byte)
#define ID_CMD_REG 0x86 // Device ID register

static const float LIGHT_GAIN = 1.0f;
static const float LIGHT_INTEGRATION_TIME = 100.0f;

static uint8_t enable_light_sensor[] = { ALS_CONTROL_REG, ALS_CONTROL_CMD };
static uint8_t config_light_sensor[] = { ALS_MEAS_RATE_REG, ALS_MEAS_RATE_CMD };

typedef struct {
    uint16_t ch0; // Visible + IR
    uint16_t ch1; // IR only
    float lux; // Calculated lux value
} LightSensorData;

void light_sensor_init(void);
void light_sensor_read(LightSensorData *data);
float light_lux(const LightSensorData  *data);
void light_whoami(void);

#endif
