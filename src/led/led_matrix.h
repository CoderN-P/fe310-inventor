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
#define MIN_BRIGHTNESS 20
#define MAX_BRIGHTNESS 255

#define LED_MATRIX_UPDATE_INTERVAL 50 // ms


void disable_interrupts(void);
void enable_interrupts(void);
void gpio_direct_init(void);
void gpio_high(void);
void gpio_low(void);

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
void send_color(Color color);
void clear(void);
void show();
uint8_t brightness_from_lux(float lux);
void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);
void bubble_level(float pitch, float roll, float lux);
Color hsv_to_rgb(uint8_t h, uint8_t s, uint8_t v);
void clear_led_matrix(void);
void compass(int heading, Color color);
#endif // LED_MATRIX_H