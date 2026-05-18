#ifndef LED_MATRIX_H
#define LED_MATRIX_H

#include <stdint.h>
#include <metal/cpu.h>

#define LED_MATRIX_GPIO 22 // GPIO pin for LED matrix
#define LED_WIDTH 8 // Width of the LED matrix
#define LED_HEIGHT 6 // Height of the LED matrix
#define NUM_LEDS (LED_WIDTH * LED_HEIGHT) // Total number of LEDs in the matrix
#define GPIO_BASE        0x10012000UL // Base memory address for GPIO registers
#define GPIO_OUTPUT_VAL_OFFSET 0x0C
#define PIN_MASK (1 << LED_MATRIX_GPIO) // Mask for LED matrix GPIO pin

#define MIN_LUX  10      // darkest room
#define MAX_LUX  3000    // under desk lamp or bright window
#define MIN_BRIGHTNESS 5
#define MAX_BRIGHTNESS 255

#define LED_MATRIX_UPDATE_INTERVAL 50 // ms


void disable_interrupts(void);
void enable_interrupts(void);
void gpio_direct_init(void);

/**

 * @brief Sends a WS2812 "0" bit (Short High, Long Low) at 150 MHz CPU clock
 * T0H Target: ~300ns (45 cycles) | T0L Target: ~900ns (135 cycles)
 */
static __attribute__((aligned(64), always_inline)) inline void gpio_low(void) {
    register uint32_t base = GPIO_BASE;
    register uint32_t mask = PIN_MASK;
    register uint32_t offset = GPIO_OUTPUT_VAL_OFFSET;

    asm volatile (
            "add  t0, %0, %2\n\t"    // Pre-calculate target memory address

            // --- START CRITICAL TIMING ---
            "sw   %1, 0(t0)\n\t"     // Pin HIGH -> Starts T0H (takes ~3 cycles)
            ".rept 42\n\t"           // Pad T0H to 45 cycles total
            "nop\n\t"
            ".endr\n\t"

            "sw   zero, 0(t0)\n\t"   // Pin LOW -> Ends T0H, Starts T0L (takes ~3 cycles)
            ".rept 132\n\t"          // Pad T0L to 135 cycles total
            "nop\n\t"
            ".endr\n\t"
            :
            : "r" (base), "r" (mask), "r" (offset)
            : "t0"
            );
}

/**
 * @brief Sends a WS2812 "1" bit (Long High, Short Low) at 150 MHz CPU clock
 * T1H Target: ~900ns (135 cycles) | T1L Target: ~300ns (45 cycles)
 */
static __attribute__((aligned(64), always_inline)) inline void gpio_high(void) {
    register uint32_t base = GPIO_BASE;
    register uint32_t mask = PIN_MASK;
    register uint32_t offset = GPIO_OUTPUT_VAL_OFFSET;

    asm volatile (
            "add  t0, %0, %2\n\t"    // Pre-calculate target memory address

            // --- START CRITICAL TIMING ---
            "sw   %1, 0(t0)\n\t"     // Pin HIGH -> Starts T1H (takes ~3 cycles)
            ".rept 132\n\t"          // Pad T1H to 135 cycles total
            "nop\n\t"
            ".endr\n\t"

            "sw   zero, 0(t0)\n\t"   // Pin LOW -> Ends T1H, Starts T1L (takes ~3 cycles)
            ".rept 42\n\t"           // Pad T1L to 45 cycles total
            "nop\n\t"
            ".endr\n\t"
            :
            : "r" (base), "r" (mask), "r" (offset)
            : "t0"
            );
}

typedef struct Color {
    uint8_t r; // Red component (0-255)
    uint8_t g; // Green component (0-255)
    uint8_t b; // Blue component (0-255)
} Color;

Color led_matrix[LED_WIDTH*LED_HEIGHT];
uint64_t last_led_update_time; // Last time the LED matrix was updated


extern struct metal_cpu *cpu; // Global CPU pointer for direct timer access
void send_bit(uint8_t bit);
void send_byte(uint8_t byte);
void send_color(uint8_t r, uint8_t g, uint8_t b);
void clear(void);
void show();
uint8_t brightness_from_lux(float lux);
void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);
void bubble_level(float pitch_raw, float roll_raw, float lux);
Color hsv_to_rgb(uint8_t h, uint8_t s, uint8_t v);
void clear_led_matrix(void);
void compass(int heading, Color color);
void fluid_sim(float pitch, float roll, float lux);
void test(float lux);
#endif // LED_MATRIX_H
