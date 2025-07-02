#include "light.h"
#include <stdio.h>
#include <metal/i2c.h>

void light_sensor_init() {
    metal_i2c_write(metal_i2c_get_device(0), LIGHT_I2C_ADDR, 2, enable_light_sensor, METAL_I2C_STOP_ENABLE);
    metal_i2c_write(metal_i2c_get_device(0), LIGHT_I2C_ADDR, 2, config_light_sensor, METAL_I2C_STOP_ENABLE);
}

void light_sensor_read(LightSensorData *data) {
    uint8_t reg;
    uint8_t raw_data[4];  // [0]=CH1_L, [1]=CH1_H, [2]=CH0_L, [3]=CH0_H

    // Read CH1 low
    reg = ALS_DATA_CH1_LOW;
    metal_i2c_write(metal_i2c_get_device(0), LIGHT_I2C_ADDR, 1, &reg, METAL_I2C_STOP_DISABLE);
    metal_i2c_read(metal_i2c_get_device(0), LIGHT_I2C_ADDR, 1, &raw_data[0], METAL_I2C_STOP_ENABLE);

    // Read CH1 high
    reg = ALS_DATA_CH1_HIGH;
    metal_i2c_write(metal_i2c_get_device(0), LIGHT_I2C_ADDR, 1, &reg, METAL_I2C_STOP_DISABLE);
    metal_i2c_read(metal_i2c_get_device(0), LIGHT_I2C_ADDR, 1, &raw_data[1], METAL_I2C_STOP_ENABLE);

    // Read CH0 low
    reg = ALS_DATA_CH0_LOW;
    metal_i2c_write(metal_i2c_get_device(0), LIGHT_I2C_ADDR, 1, &reg, METAL_I2C_STOP_DISABLE);
    metal_i2c_read(metal_i2c_get_device(0), LIGHT_I2C_ADDR, 1, &raw_data[2], METAL_I2C_STOP_ENABLE);

    // Read CH0 high
    reg = ALS_DATA_CH0_HIGH;
    metal_i2c_write(metal_i2c_get_device(0), LIGHT_I2C_ADDR, 1, &reg, METAL_I2C_STOP_DISABLE);
    metal_i2c_read(metal_i2c_get_device(0), LIGHT_I2C_ADDR, 1, &raw_data[3], METAL_I2C_STOP_ENABLE);

    data->ch1 = ((uint16_t)raw_data[1] << 8) | raw_data[0];  // IR
    data->ch0 = ((uint16_t)raw_data[3] << 8) | raw_data[2];  // Vis + IR
    data->lux = light_lux(data);
}

float light_lux(const LightSensorData *data){
    if (data->ch0 == 0 || data->ch0 < data->ch1) return 0.0f;

    float cpl = (LIGHT_INTEGRATION_TIME * LIGHT_GAIN) / 408.0f;
    float lux = ((float)data->ch0 - (float)data->ch1) * (1.0f - ((float)data->ch1 / (float)data->ch0)) / cpl;
    return lux > 0 ? lux : 0.0f;
}

void light_whoami(){
    uint8_t reg = ID_CMD_REG;
    uint8_t id_data[1];

    metal_i2c_write(metal_i2c_get_device(0), LIGHT_I2C_ADDR, 1, &reg, METAL_I2C_STOP_DISABLE);
    metal_i2c_read(metal_i2c_get_device(0), LIGHT_I2C_ADDR, 1, id_data, METAL_I2C_STOP_ENABLE);

    printf("Light Sensor ID: 0x%02X\n", id_data[0]);
}
