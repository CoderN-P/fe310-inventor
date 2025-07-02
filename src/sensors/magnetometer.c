#include "magnetometer.h"
#include "accelerometer.h"
#include <metal/i2c.h>
#include <math.h>
#include <stdio.h>
#include "../utils/delay.h"

void magnetometer_init() {
    metal_i2c_write(metal_i2c_get_device(0), MAGNETOMETER_I2C_ADDR, 2, magnetometer_config, METAL_I2C_STOP_ENABLE);
    reset_calibration();
}

void magnetometer_read(MagnetometerData *data, const AccelerometerData *accel_data) {
    uint8_t reg = DATA_REG_M; // Auto-increment bit set
    uint8_t raw_data[6];

    metal_i2c_write(metal_i2c_get_device(0), MAGNETOMETER_I2C_ADDR, 1, &reg, METAL_I2C_STOP_DISABLE);
    metal_i2c_read(metal_i2c_get_device(0), MAGNETOMETER_I2C_ADDR, 6, raw_data, METAL_I2C_STOP_ENABLE);

    data->x = (int16_t)(raw_data[1] << 8 | raw_data[0]);
    data->y = (int16_t)(raw_data[3] << 8 | raw_data[2]);
    data->z = (int16_t)(raw_data[5] << 8 | raw_data[4]);

    data->x = float_to_q8_7(data->x * MT_LSB); // Convert to microteslas (µT)
    data->y = float_to_q8_7(data->y * MT_LSB); // Convert to microteslas (µT)
    data->z = float_to_q8_7(data->z * MT_LSB); // Convert to microteslas (µT)

    data->angle = magnetometer_angle(data, accel_data);
}

MagnetometerAngleData magnetometer_angle(const MagnetometerData *mag, const AccelerometerData *accel) {
    // Normalize accelerometer vector
    float norm = sqrtf(accel->x * accel->x + accel->y * accel->y + accel->z * accel->z);
    float ax = accel->x / norm;
    float ay = accel->y / norm;
    float az = accel->z / norm;

    // Compute pitch and roll (custom definition)
    float pitch = atan2f(ax, sqrtf(ay * ay + az * az));
    float roll  = atan2f(-ay, sqrtf(ax * ax + az * az));

    // Flip pitch if upside down (optional)
    if (az < 0) pitch = -pitch;

    // Clamp pitch and roll (optional - adjust to your needs)
    pitch = -fmaxf(fminf(pitch, M_PI / 12), -M_PI / 12); // ±15°
    roll  = -fmaxf(fminf(roll,  M_PI / 6),  -M_PI / 6);  // ±30°

    // Tilt-compensate magnetometer readings
    float mx = q8_7_to_float(mag->x);
    float my = q8_7_to_float(mag->y);
    float mz = q8_7_to_float(mag->z);

    // Apply calibration

    float cal_x, cal_y, cal_z;
    calibrate_raw_data(mx, my, mz, &cal_x, &cal_y, &cal_z);

    float mx2 = cal_x * cosf(pitch) + cal_z * sinf(pitch);
    float my2 = cal_x * sinf(roll) * sinf(pitch)
                + cal_y * cosf(roll)
                - cal_z * sinf(roll) * cosf(pitch);

    // Compute heading from tilt-compensated values
    float heading_rad = atan2f(-my2, mx2);
    if (heading_rad < 0) heading_rad += 2.0f * M_PI;
    int16_t heading_deg = (int16_t)(heading_rad * 180.0f / M_PI) + MAG_DECLINATION_DEG;


    return (MagnetometerAngleData){
            .heading = heading_deg,
            .pitch = pitch * 180.0f / M_PI,
            .roll  = roll  * 180.0f / M_PI
    };
}

void reset_calibration(){
    uint8_t reset_data[] = {
            OFFSET_X_REG_L_M,
            0x00, // Low byte
            0x00, // High byte
            0x00, // Low byte
            0x00, // High byte
            0x00, // Low byte
            0x00  // High byte
    };

    metal_i2c_write(metal_i2c_get_device(0), MAGNETOMETER_I2C_ADDR, sizeof(reset_data), reset_data, METAL_I2C_STOP_ENABLE);
}

void calibrate_magnetometer(){
    reset_calibration();

    MagnetometerData raw_data;
    AccelerometerData accel_data;
    accelerometer_read(&accel_data);
    magnetometer_read(&raw_data, &accel_data);
    printf("%d, %d, %d\n", raw_data.x, raw_data.y, raw_data.z);
    delay_ms(30);
}

void calibrate_raw_data(float raw_x, float raw_y, float raw_z, float *cal_x, float *cal_y, float *cal_z) {
        // Hard iron correction
        float mx = raw_x - 42.683393f;
        float my = raw_y - (-20.579634f);
        float mz = raw_z - 25.621804f;

        // Soft iron correction (matrix multiplication)
        *cal_x = 1.165877f * mx + 0.035179f * my + 0.010642f * mz;
        *cal_y = 0.035179f * mx + 1.036364f * my + -0.042993f * mz;
        *cal_z = 0.010642f * mx + -0.042993f * my + 0.985961f * mz;
}