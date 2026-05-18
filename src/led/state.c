#include "state.h"
#include "button.h"
#include <stdbool.h>
#include "led_matrix.h"

enum LED_STATE current_state = BUBBLE_LEVEL; // Default state

void handle_state(){
    bool current_button_a_state = button_a_pressed();
    bool current_button_b_state = button_b_pressed();
    if (current_button_b_state && !previous_button_b_state) {
        if (current_state == BUBBLE_LEVEL) {
            current_state = COMPASS;
        } else if (current_state == COMPASS) {
            current_state = FLUID_SIM;
        } else if (current_state == FLUID_SIM) {
            current_state = TEST;
        } else if (current_state == TEST){
            current_state = CALIBRATION;
        } else if (current_state == CALIBRATION) {
            current_state = OFF;
        } else {
            current_state = BUBBLE_LEVEL;
        }
        clear();
    } else if (current_button_a_state && !previous_button_a_state) {
        if (current_state == BUBBLE_LEVEL) {
            current_state = OFF;
        } else if (current_state == COMPASS) {
            current_state = BUBBLE_LEVEL;
        } else if (current_state == FLUID_SIM) {
            current_state = COMPASS;
        } else if (current_state == TEST) {
            current_state = FLUID_SIM;
        } else if (current_state == CALIBRATION){
            current_state = TEST;
        } else {
            current_state = CALIBRATION;
        }
        clear();
    }


    previous_button_a_state = current_button_a_state;
    previous_button_b_state = current_button_b_state;
}
