#include "temperature.h"
#include <metal/i2c.h>

void temperature_init() {
    metal_i2c_write(metal_i2c_get_device(0), TEMPERATURE_I2C_ADDR, 2, temperature_config, METAL_I2C_STOP_ENABLE);
}

void temperature_read(TemperatureData *data) {
    uint8_t reg = DATA_REG_T; // Auto-increment bit set
    uint8_t raw_data[2];

    metal_i2c_write(metal_i2c_get_device(0), TEMPERATURE_I2C_ADDR, 1, &reg, METAL_I2C_STOP_DISABLE);
    metal_i2c_read(metal_i2c_get_device(0), TEMPERATURE_I2C_ADDR, 2, raw_data, METAL_I2C_STOP_ENABLE);

    int16_t raw_temp = (int16_t) raw_data[1];

    // Convert to °C: (raw / 8.0) + 20°C offset
    data->temperature = raw_temp;
}
