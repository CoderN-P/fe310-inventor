#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>
#include <stdbool.h>

#define BUTTON_A_PIN 11
#define BUTTON_B_PIN 0


void init_buttons(void);
bool button_a_pressed(void);
bool button_b_pressed(void);

#endif // BUTTON_H