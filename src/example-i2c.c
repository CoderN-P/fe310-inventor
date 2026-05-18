#include <stdio.h>
#include <stdint.h>
#include <metal/i2c.h>
#include <metal/cpu.h>
#include <metal/gpio.h>
#include "sensors/sensors.h"
#include "utils/uart.h"
#include "utils/delay.h"
#include "led/led_matrix.h"
#include "led/state.h"
#include "led/button.h"
#include "led/fluid_sim.h"
#include <time.h>


#define OK 0
#define ERR -1
#define TARGET_FPS 15
#define FRAME_TIME (1000 / TARGET_FPS) // Target frame time in milliseconds

extern struct metal_clock __metal_dt_clock_4;

int8_t init(){
    struct metal_cpu *cpu = metal_cpu_get(metal_cpu_get_current_hartid());
    struct metal_interrupt *cpu_intr = metal_cpu_interrupt_controller(cpu);
    metal_interrupt_init(cpu_intr);
    struct metal_i2c *i2c = metal_i2c_get_device(0);

    long actual_hz = metal_clock_set_rate_hz(&__metal_dt_clock_4, 150000000); // Set CPU clock to 150 MHz for better performance

    if (actual_hz != 150000000) {
        printf("Warning: Requested CPU clock rate of 150 MHz, but got %ld Hz.\n", actual_hz);
    }

    if (!i2c) {
        printf("Failed to get I2C device.\n");
        return ERR;
    }

    metal_i2c_init(i2c, 100000, METAL_I2C_MASTER);
    init_uart();
    sensors_init();
    delay_ms(1000); // Allows sensors to initialize properly

    return OK;
}


int main() {
    if (init() != OK) {
        printf("Initialization failed.\n");
        return ERR;
    }

    SensorData sensor_data;

    init_buttons();
    initFluidSim();
    gpio_direct_init();
    delay_ms(1000); // Allow GPIO setup to stabilize
    calibrate_magnetometer();

    while (1) {
        clock_t start_time = clock();
        read_data(&sensor_data); // Read data from UART and process commands
        sensors_read(&sensor_data);

        disable_interrupts();
        if (current_state == BUBBLE_LEVEL){
            bubble_level(sensor_data.magnetometer.angle.pitch, sensor_data.magnetometer.angle.roll, sensor_data.light.lux);
        } else if (current_state == COMPASS) {
            compass(sensor_data.magnetometer.angle.heading, (Color) {0, 15, 30});
        } else if (current_state == FLUID_SIM){
            fluid_sim(sensor_data.magnetometer.angle.pitch, sensor_data.magnetometer.angle.roll, sensor_data.light.lux);
        } else if (current_state == TEST){
            test(sensor_data.light.lux);
        } else if (current_state == CALIBRATION){
            enable_interrupts();
            calibrate_magnetometer();
        }
        enable_interrupts();

        handle_state();

        // Measure elapsed time
        clock_t end_time = clock();
        int elapsed_time = (end_time - start_time) * 1000 / CLOCKS_PER_SEC;

        // Delay to maintain 30 FPS
        if (elapsed_time < FRAME_TIME) {
            delay_ms(FRAME_TIME - elapsed_time);
        }

    }

    return OK;
}
