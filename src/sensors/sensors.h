#ifndef SENSORS_H
#define SENSORS_H

#include "accelerometer.h"
#include "temperature.h"
#include "light.h"
#include "magnetometer.h"
#define CPU_FREQ_HZ 16000000 // 16 MHz CPU frequency

#define ACCEL_UPDATE_INTERVAL 3 // ms
#define MAGNETOMETER_UPDATE_INTERVAL 10 // ms
#define TEMPERATURE_UPDATE_INTERVAL 1000 // ms
#define LIGHT_SENSOR_UPDATE_INTERVAL 250 // ms

#define ACCELEROMETER_SMOOTHING_WINDOW 5 // Number of samples for smoothing

typedef struct SensorData {
    AccelerometerData accelerometer;
    TemperatureData temperature;
    LightSensorData light;
    MagnetometerData magnetometer;
} SensorData;

uint64_t lastAccelerometerUpdateTime;
uint64_t lastMagnetometerUpdateTime;
uint64_t lastLightSensorUpdateTime;
uint64_t lastTemperatureUpdateTime;

struct metal_cpu *cpu;

uint8_t accelerometer_read_idx;
static float prev_lux = -1.0f;
static int valid_accel_samples = 0;
static AccelerometerData accelerometer_data_buffer[ACCELEROMETER_SMOOTHING_WINDOW] = {
        (AccelerometerData){
                .x = 0,
                .y = 0,
                .z = 0
        }
};
int32_t accelerometer_smoothing_sum_x,
         accelerometer_smoothing_sum_y,
         accelerometer_smoothing_sum_z;


void sensors_init(void);
void sensors_read(SensorData *data);
AccelerometerData accel_running_avg(AccelerometerData new_data);

#endif