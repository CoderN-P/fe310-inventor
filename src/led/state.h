#ifndef STATE_H
#define STATE_H

#include <stdbool.h>


enum LED_STATE {
    BUBBLE_LEVEL,
    COMPASS,
    CALIBRATION,
    OFF,
};

extern enum LED_STATE current_state; // Default state
static bool previous_button_a_state = false;
static bool previous_button_b_state = false;
void handle_state(void);

#endif // STATE_H