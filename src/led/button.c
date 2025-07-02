#include "button.h"
#include <metal/gpio.h>
#include <stdbool.h>


void init_buttons(){
    struct metal_gpio *gpio = metal_gpio_get_device(0);
    metal_gpio_disable_output(gpio, BUTTON_A_PIN);
    metal_gpio_enable_input(gpio, BUTTON_A_PIN);
    metal_gpio_disable_output(gpio, BUTTON_B_PIN);
    metal_gpio_enable_input(gpio, BUTTON_B_PIN);
}

bool button_a_pressed() {
    struct metal_gpio *gpio = metal_gpio_get_device(0);
    return !metal_gpio_get_input_pin(gpio, BUTTON_A_PIN);
}

bool button_b_pressed() {
    struct metal_gpio *gpio = metal_gpio_get_device(0);
    return !metal_gpio_get_input_pin(gpio, BUTTON_B_PIN);
}