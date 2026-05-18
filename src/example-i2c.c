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


int8_t init(){
    struct metal_cpu *cpu = metal_cpu_get(metal_cpu_get_current_hartid());
    struct metal_interrupt *cpu_intr = metal_cpu_interrupt_controller(cpu);
    metal_interrupt_init(cpu_intr);
    struct metal_i2c *i2c = metal_i2c_get_device(0);

    long actual_hz = metal_clock_set_rate_hz(&__metal_dt_clock_4, 160000000); // Set CPU clock to 160 MHz for better performance

    if (actual_hz != 160000000) {
        printf("Warning: Requested CPU clock rate of 160 MHz, but got %ld Hz.\n", actual_hz);
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

    while (1) {
        read_data(&sensor_data); // Read data from UART and process commands
        sensors_read(&sensor_data);

        disable_interrupts();

        if (current_state == BUBBLE_LEVEL){
            bubble_level(sensor_data.magnetometer.angle.pitch, sensor_data.magnetometer.angle.roll, sensor_data.light.lux);
        } else if (current_state == COMPASS) {
            compass(sensor_data.magnetometer.angle.heading, (Color) {0, 100, 200}, sensor_data.light.lux);
        } else if (current_state == FLUID_SIM){
            fluid_sim(sensor_data.magnetometer.angle.pitch, sensor_data.magnetometer.angle.roll, sensor_data.light.lux);
        } else if (current_state == TEST){
            test(sensor_data.light.lux);
        } else if (current_state == CALIBRATION){
            enable_interrupts();
            calibrate_magnetometer();
        } else if (current_state == OFF){
            clear();
        }

        enable_interrupts();


        handle_state();
    }



    return OK;
}
