//
// Created by Neel P on 6/19/25.
//

#ifndef DELAY_H
#define DELAY_H

#include <stdint.h>

#define MTIME_FREQ 16000000  // 16 MHz default HFROSC
#define CPU_FREQ_HZ 16000000  // 16 MHz HFROSC

void delay_ms(uint32_t ms);
void delay_us(uint32_t us);

#endif
