#include <metal/cpu.h>
#include <stdio.h>
#include "sensors.h"
#include "accelerometer.h"
#include "temperature.h"
#include "light.h"
#include "magnetometer.h"

void sensors_init() {
    cpu = metal_cpu_get(metal_cpu_get_current_hartid());
    accelerometer_init();
    magnetometer_init();
    light_sensor_init();
    temperature_init();
}

AccelerometerData accel_running_avg(AccelerometerData new_data){
    valid_accel_samples = (valid_accel_samples < ACCELEROMETER_SMOOTHING_WINDOW) ? valid_accel_samples + 1 : ACCELEROMETER_SMOOTHING_WINDOW;

    accelerometer_smoothing_sum_x -= accelerometer_data_buffer[accelerometer_read_idx].x;
    accelerometer_smoothing_sum_y -= accelerometer_data_buffer[accelerometer_read_idx].y;
    accelerometer_smoothing_sum_z -= accelerometer_data_buffer[accelerometer_read_idx].z;
    accelerometer_data_buffer[accelerometer_read_idx] = new_data;
    accelerometer_smoothing_sum_x += new_data.x;
    accelerometer_smoothing_sum_y += new_data.y;
    accelerometer_smoothing_sum_z += new_data.z;
    accelerometer_read_idx = (accelerometer_read_idx + 1) % ACCELEROMETER_SMOOTHING_WINDOW;

    if (valid_accel_samples < ACCELEROMETER_SMOOTHING_WINDOW) {
        return new_data; // Not enough samples yet, return the new data directly
    }

    return (AccelerometerData){
        .x = (q4_11_t)(accelerometer_smoothing_sum_x / ACCELEROMETER_SMOOTHING_WINDOW),
        .y = (q4_11_t)(accelerometer_smoothing_sum_y / ACCELEROMETER_SMOOTHING_WINDOW),
        .z = (q4_11_t)(accelerometer_smoothing_sum_z / ACCELEROMETER_SMOOTHING_WINDOW)
    };
}

void sensors_read(SensorData *data) {
    uint64_t now = metal_cpu_get_timer(cpu);
    uint64_t accel_ticks = ((uint64_t)ACCEL_UPDATE_INTERVAL * CPU_FREQ_HZ) / 1000;
    uint64_t mag_ticks = ((uint64_t)MAGNETOMETER_UPDATE_INTERVAL * CPU_FREQ_HZ) / 1000;
    uint64_t light_ticks = ((uint64_t)LIGHT_SENSOR_UPDATE_INTERVAL * CPU_FREQ_HZ) / 1000;
    uint64_t temp_ticks = ((uint64_t)TEMPERATURE_UPDATE_INTERVAL * CPU_FREQ_HZ) / 1000;

    if (now - lastAccelerometerUpdateTime >= accel_ticks) {
        lastAccelerometerUpdateTime = now;
        AccelerometerData accel_data;
        accelerometer_read(&accel_data);
        data->accelerometer = accel_running_avg(accel_data);
    }

    if (now - lastMagnetometerUpdateTime >= mag_ticks) {
        lastMagnetometerUpdateTime = now;
        magnetometer_read(&data->magnetometer, &data->accelerometer);
    }

    if (now - lastLightSensorUpdateTime >= light_ticks) {
        lastLightSensorUpdateTime = now;
        light_sensor_read(&data->light);

        if (prev_lux < 0) {
            prev_lux = data->light.lux;
        } else {
            // Simple smoothing: average with previous value
            data->light.lux = (prev_lux*0.8f + data->light.lux*0.2f);
            prev_lux = data->light.lux;
        }
    }

    if (now - lastTemperatureUpdateTime >= temp_ticks) {
        lastTemperatureUpdateTime = now;
        temperature_read(&data->temperature);
    }
}

