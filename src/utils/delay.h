//
// Created by Neel P on 6/19/25.
//

#ifndef DELAY_H
#define DELAY_H

#include <stdint.h>

extern struct metal_clock __metal_dt_clock_4;

void delay_ms(uint32_t ms);
void delay_us(uint32_t us);

#endif
