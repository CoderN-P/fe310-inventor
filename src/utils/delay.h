//
// Created by Neel P on 6/19/25.
//

#ifndef DELAY_H
#define DELAY_H

#include <stdint.h>

#define MTIME_FREQ_HZ 32768

void delay_ms(uint32_t ms);
void delay_us(uint32_t us);

#endif
