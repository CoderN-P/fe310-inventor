#include "accelerometer.h"
#include <metal/i2c.h>

void accelerometer_init(){
    metal_i2c_write(metal_i2c_get_device(0), ACCELEROMETER_I2C_ADDR, sizeof(accelerometer_config), accelerometer_config, METAL_I2C_STOP_ENABLE);
    metal_i2c_write(metal_i2c_get_device(0), ACCELEROMETER_I2C_ADDR, sizeof(accelerometer_config_2), accelerometer_config_2, METAL_I2C_STOP_ENABLE);
}

void accelerometer_read(AccelerometerData *data) {
    uint8_t reg = DATA_REG_A; // 0x28 with auto-increment (0x80)
    uint8_t raw_data[6];

    metal_i2c_write(metal_i2c_get_device(0), ACCELEROMETER_I2C_ADDR, 1, &reg, METAL_I2C_STOP_DISABLE);
    metal_i2c_read(metal_i2c_get_device(0), ACCELEROMETER_I2C_ADDR, 6, raw_data, METAL_I2C_STOP_ENABLE);

    // Proper signed 16-bit conversion (little-endian: LSB first)
    int16_t raw_x = ((int16_t)((uint16_t)raw_data[1] << 8) | (uint16_t)raw_data[0]) >> 4; // 12 bit resolution right justified
    int16_t raw_y = ((int16_t)((uint16_t)raw_data[3] << 8) | (uint16_t)raw_data[2]) >> 4; // 12 bit resolution right justified
    int16_t raw_z = ((int16_t)((uint16_t)raw_data[5] << 8) | (uint16_t)raw_data[4]) >> 4; // 12 bit resolution right justified

    // Convert to g using float scaling
    data->x = float_to_q4_11(raw_x * SCALE_FACTOR_8G);
    data->y = float_to_q4_11(raw_y * SCALE_FACTOR_8G);
    data->z = float_to_q4_11(raw_z * SCALE_FACTOR_8G);
}

