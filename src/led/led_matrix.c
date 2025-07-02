#include "led_matrix.h"
#include "../sensors/accelerometer.h"
#include "../utils/delay.h"
#include "./compass.h"
#include <stdio.h>
#include <math.h>
#include <metal/cpu.h>
#include <stdint.h>



void disable_interrupts() {
    asm volatile("csrrc x0, mstatus, %0" :: "r"(0x8)); // Clear MIE bit in mstatus
}

void enable_interrupts() {
    asm volatile("csrrs x0, mstatus, %0" :: "r"(0x8)); // Set MIE bit in mstatus
}

void gpio_low(void) {
    asm volatile (
            "li t0, %0\n\t"
            "li t1, %1\n\t"
            "sw t1, %2(t0)\n\t"     // Set HIGH
            "nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t"  // 5 NOPs
            "sw zero, %2(t0)\n\t"   // Set LOW
            "nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t"  // 12 NOPs
            "nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t"
            :
            : "i" (GPIO_BASE), "i" (PIN_MASK), "i" (GPIO_OUTPUT_VAL_OFFSET)
            : "t0", "t1"
            );
}

void gpio_high(void) {
    asm volatile (
            "li t0, %0\n\t"
            "li t1, %1\n\t"
            "sw t1, %2(t0)\n\t"     // Set HIGH
            "nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t"  // 12 NOPs
            "nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t"
            "sw zero, %2(t0)\n\t"   // Set LOW
            "nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t"  // 5 NOPs
            :
            : "i" (GPIO_BASE), "i" (PIN_MASK), "i" (GPIO_OUTPUT_VAL_OFFSET)
            : "t0", "t1"
            );
}

// Don't forget to enable GPIO 22 as output first
void gpio_direct_init(void) {
    volatile uint32_t *gpio_output_en = (uint32_t*)(GPIO_BASE + 0x08);
    *gpio_output_en |= PIN_MASK;  // Enable pin 22 as output
}


void send_bit(uint8_t bit) {
    if (bit) {
        gpio_high(); // Set GPIO high
    } else {
        gpio_low();
    }
}

void send_byte(uint8_t byte) {
    for (int i = 0; i < 8; i++) {
        send_bit((byte >> (7 - i)) & 1);
    }
}

void send_color(Color color) {
    // Send color data in GRB format
    send_byte(color.g);
    send_byte(color.r);
    send_byte(color.b);
}

void clear() {
    // Clear the LED matrix by sending black color to all LEDs
    for (int i = 0; i < NUM_LEDS; i++) {
        send_color((Color){0, 0, 0});
    }
    gpio_low(); // Ensure GPIO is low after sending
    delay_us(500); // Wait for 500us to latch the data
}

void show() {
    // Send color data for each LED in the matrix
    disable_interrupts();
    for (int i = 0; i < LED_HEIGHT*LED_WIDTH; i++) {
        send_color(led_matrix[i]);
    }
    gpio_low(); // Ensure GPIO is low after sending
    delay_us(1000); // Wait for 500us to latch the data
    enable_interrupts();
}


void clear_led_matrix() {
    // Clear the LED matrix by setting all pixels to black
    for (int i = 0; i < LED_HEIGHT*LED_WIDTH; i++) {
        led_matrix[i] = (Color){0, 0, 0}; // Set each pixel to black
    }
}

void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (x < 0 || x >= LED_WIDTH || y < 0 || y >= LED_HEIGHT) {
        return; // Out of bounds
    }
    led_matrix[y*LED_WIDTH + LED_WIDTH-1-x] = (Color){r, g, b}; // Set pixel color
}

void draw_bubble(int l, int r, int t, int b, Color color) {
    // Draw a bubble shape on the LED matrix
    set_pixel(l, t, color.r, color.g, color.b); // Top left
    set_pixel(r, t, color.r, color.g, color.b); // Top right
    set_pixel(l, b, color.r, color.g, color.b); // Bottom left
    set_pixel(r, b, color.r, color.g, color.b); // Bottom right
}
void bubble_level(float pitch, float roll, float lux) {
    // Clear the LED matrix
    clear_led_matrix();
    // Calculate how level the device is
    float total_tilt = sqrt(pitch*pitch + roll*roll);

    // Map angles to bubble position
// Center is at (LED_WIDTH/2, LED_HEIGHT/2)
// ±30 degrees maps to ±2-3 pixels from center
    int bubble_center_x = LED_WIDTH/2 + (int)(roll * (LED_WIDTH-2) / 60.0f);
    int bubble_center_y = LED_HEIGHT/2 + (int)(pitch * (LED_HEIGHT-2) / 30.0f);

    int bubble_left = bubble_center_x - 1;
    int bubble_right = bubble_center_x;
    int bubble_top = bubble_center_y - 1;
    int bubble_bottom = bubble_center_y;

    if (bubble_left < 0) bubble_left = 0;
    if (bubble_right >= LED_WIDTH) bubble_right = LED_WIDTH - 1;
    if (bubble_top < 0) bubble_top = 0;
    if (bubble_bottom >= LED_HEIGHT) bubble_top = LED_HEIGHT - 1;

    if (total_tilt < 1.0f){
        // fill green
        for (int i = 0; i < LED_HEIGHT*LED_WIDTH; i++) {
            led_matrix[i] = (Color){0, 10, 0}; // Fill the matrix with green
        }
    } else if (total_tilt < 5.0f) {
        draw_bubble(bubble_left, bubble_right, bubble_top, bubble_bottom, (Color){0, 10, 0}); // Green
    } else if (total_tilt < 15.0f) {
        draw_bubble(bubble_left, bubble_right, bubble_top, bubble_bottom, (Color){10, 10, 0}); // Yellow
    } else if (total_tilt < 25.0f) {
        draw_bubble(bubble_left, bubble_right, bubble_top, bubble_bottom, (Color){10, 5, 0}); // Orange
    } else {
        draw_bubble(bubble_left, bubble_right, bubble_top, bubble_bottom, (Color){10, 0, 0}); // Red
    }

    show(); // Update the LED matrix display
}

void compass(int heading, Color color) {
    // Clear the LED matrix

    clear_led_matrix();

    // Normalize heading to 0-360 degrees
    if (heading < 0) heading += 360;
    if (heading >= 360) heading -= 360;

    // Calculate the index of the arrow based on the heading
    int arrow_index = ((heading % 360) + 22) / 45 % 8;
    // Draw the arrow for the current heading
    for (int i = 0; i < get_arrow_point_count(arrow_index); i++) {
        point_t point = get_arrow_point(arrow_index, i);
        set_pixel(point.x, point.y, color.r, color.g, color.b);
    }

    show(); // Update the LED matrix display
}

uint8_t brightness_from_lux(float lux) {
    if (lux < MIN_LUX) lux = MIN_LUX;
    if (lux > MAX_LUX) lux = MAX_LUX;

    float norm = (lux - MIN_LUX) / (MAX_LUX - MIN_LUX);
    return MIN_BRIGHTNESS + (uint8_t)(norm * (MAX_BRIGHTNESS - MIN_BRIGHTNESS));
}

Color hsv_to_rgb(uint8_t h, uint8_t s, uint8_t v) {
    uint8_t region = h / 43;
    uint8_t remainder = (h - (region * 43)) * 6;

    uint8_t p = (v * (255 - s)) >> 8;
    uint8_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    uint8_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch (region) {
        case 0: return (Color){v, t, p};
        case 1: return (Color){q, v, p};
        case 2: return (Color){p, v, t};
        case 3: return (Color){p, q, v};
        case 4: return (Color){t, p, v};
        default: return (Color){v, p, q};
    }
}
