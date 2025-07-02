//
// Created by Neel P on 6/16/25.
//

#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <stdint.h>
#define TEMPERATURE_I2C_ADDR 0x19
#define TEMP_CFG_REG_A 0x1F
#define ENABLE_TEMP_SENSOR 0xC0 // Enable temperature sensor
#define TEMP_OUT_L_A 0x0C // Temperature output low register
#define DATA_REG_T (TEMP_OUT_L_A | 0x80) // Auto-increment bit

static uint8_t temperature_config[] = {TEMP_CFG_REG_A, ENABLE_TEMP_SENSOR};

typedef struct {
    float temperature; // Temperature in Celsius
} TemperatureData;

void temperature_init(void);
void temperature_read(TemperatureData *data);

#endif
