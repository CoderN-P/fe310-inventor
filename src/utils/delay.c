#include "delay.h"
#include <metal/cpu.h>
#include <stdint.h>


#include <metal/cpu.h>
#include <metal/clock.h>

#include <metal/cpu.h>

void delay_us(uint32_t us) {
    struct metal_cpu *cpu = metal_cpu_get(metal_cpu_get_current_hartid());
    long cpu_freq_hz = metal_clock_get_rate_hz(&__metal_dt_clock_4);
    uint64_t start = metal_cpu_get_timer(cpu);
    // minimum 1 tick to avoid zero-delay
    uint64_t ticks = ((uint64_t)us * cpu_freq_hz) / 1000000;
    if (ticks == 0) ticks = 1;
    while ((metal_cpu_get_timer(cpu) - start) < ticks);
}

void delay_ms(uint32_t ms) {
    struct metal_cpu *cpu = metal_cpu_get(metal_cpu_get_current_hartid());
    long cpu_freq_hz = metal_clock_get_rate_hz(&__metal_dt_clock_4);
    uint64_t start = metal_cpu_get_timer(cpu);
    uint64_t ticks = ((uint64_t)ms * cpu_freq_hz) / 1000;
    while ((metal_cpu_get_timer(cpu) - start) < ticks);
}