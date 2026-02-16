#include "led_matrix.h"
#include "../sensors/accelerometer.h"
#include "../utils/delay.h"
#include "./compass.h"
#include <stdio.h>
#include <math.h>
#include <metal/cpu.h>
#include <metal/gpio.h>
#include <stdint.h>
#include <stdint.h>
#include <string.h>
#include "metal/spi.h"

void disable_interrupts()
{
    asm volatile("csrrc x0, mstatus, %0" ::"r"(0x8)); // Clear MIE bit in mstatus
}

void enable_interrupts()
{
    asm volatile("csrrs x0, mstatus, %0" ::"r"(0x8)); // Set MIE bit in mstatus
}
/*
void gpio_low(void)
{
    asm volatile(
        "li t0, %0\n\t"
        "li t1, %1\n\t"
        "sw t1, %2(t0)\n\t"                               // Set HIGH
        "nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t"         // 5 NOPs
        "sw zero, %2(t0)\n\t"                             // Set LOW
        "nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t" // 12 NOPs
        "nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t"
        :
        : "i"(GPIO_BASE), "i"(PIN_MASK), "i"(GPIO_OUTPUT_VAL_OFFSET)
        : "t0", "t1");
}

void gpio_high(void)
{
    asm volatile(
        "li t0, %0\n\t"
        "li t1, %1\n\t"
        "sw t1, %2(t0)\n\t"                               // Set HIGH
        "nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t" // 12 NOPs
        "nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t"
        "sw zero, %2(t0)\n\t"                     // Set LOW
        "nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t" // 5 NOPs
        :
        : "i"(GPIO_BASE), "i"(PIN_MASK), "i"(GPIO_OUTPUT_VAL_OFFSET)
        : "t0", "t1");
}
        */
/*
void gpio_low(void)
{
    asm volatile(
        "li t0, %0\n\t"
        "li t1, %1\n\t"
        "sw t1, %2(t0)\n\t"              // Set HIGH (T0H: 220-380ns)
        ".rept 5\n\t nop\n\t .endr\n\t"  // ~312ns (within 220-380ns)
        "sw zero, %2(t0)\n\t"            // Set LOW (T0L: 580-1000ns)
        ".rept 12\n\t nop\n\t .endr\n\t" // ~750ns (within 580-1000ns)
        :
        : "i"(GPIO_BASE), "i"(PIN_MASK), "i"(GPIO_OUTPUT_VAL_OFFSET)
        : "t0", "t1");
}

void gpio_high(void)
{
    asm volatile(
        "li t0, %0\n\t"
        "li t1, %1\n\t"
        "sw t1, %2(t0)\n\t"              // Set HIGH (T1H: 580-1000ns)
        ".rept 12\n\t nop\n\t .endr\n\t" // ~750ns (within 580-1000ns)
        "sw zero, %2(t0)\n\t"            // Set LOW (T1L: 220-420ns)
        ".rept 5\n\t nop\n\t .endr\n\t"  // ~312ns (within 220-420ns)
        :
        : "i"(GPIO_BASE), "i"(PIN_MASK), "i"(GPIO_OUTPUT_VAL_OFFSET)
        : "t0", "t1");
}
*/


// Pre-calculated patterns for 2.4MHz SPI clock
const uint8_t T0H = 0b11100000; // Low bit representation
const uint8_t T1H = 0b11111100; // High bit representation

void set_pixel(int x, int y, Color c) {
    int index = y*LED_WIDTH + LED_WIDTH-1-x;
    if (index >= NUM_LEDS) return;

    // NeoPixels expect GRB order
    uint8_t colors[3] = {c.g, c.r, c.b};
    uint32_t buf_pos = index * 24;

    for (int c = 0; c < 3; c++) {
        for (int i = 7; i >= 0; i--) {
            // Map each color bit to an entire SPI byte
            spi_buffer[buf_pos++] = (colors[c] & (1 << i)) ? T1H : T0H;
        }
    }
}

void show_matrix(struct metal_spi *spi) {
    // Configure SPI for ~2.4MHz to 3MHz
    struct metal_spi_config config = {
            .protocol = METAL_SPI_SINGLE,
            .polarity = 0,
            .phase = 0,
            .little_endian = 0,
            .cs_active_high = 0,
            .csid = 0
    };

    metal_spi_init(spi, &config);
    metal_spi_set_baud_rate(spi, 2400000);
    // Send the buffer - hardware handles the clocking
    metal_spi_transfer(spi, &config, SPI_BUF_SIZE, spi_buffer, NULL);

    // Latch signal: Hold MOSI low for >50us
    // Simply wait or send a few null bytes

    delay_us(100);
}

void clear_matrix(struct metal_spi *spi) {
    // Fill the entire buffer with the '0' bit pattern (0b11100000)
    memset(spi_buffer, T0H, SPI_BUF_SIZE);

    // Push the buffer to the hardware
    show_matrix(spi);

    // Note: show_matrix handles the ~50us reset latch
    // needed to turn the LEDs off immediately.
}


// Don't forget to enable GPIO 22 as output first
void matrix_hardware_init() {
    struct metal_gpio *gpio = metal_gpio_get_device(0);
    if (gpio) {
        // Pin 22 is GPIO 22 on the FE310
        // Enable Internal I/O Function 0 (SPI1 MOSI)
        metal_gpio_enable_pinmux(gpio, 22, 0);
    }
}



void test_matrix()
{
    // Display alternating red, white, blue strips
    struct metal_spi *spi = metal_spi_get_device(2); // SPI1 controls the matrix
    if (spi == NULL) {
        return;
    }
    clear_matrix(spi);
    for (int r = 0; r < LED_HEIGHT; r++)
    {
        for (int c = 0; c < LED_WIDTH; c++)
        {
            if (r % 3 == 0)
            {
                set_pixel(c, r, (Color) {10, 0, 0});
            }
            else if (r % 3 == 1)
            {
                set_pixel(c, r, (Color) {10, 10, 10});
            }
            else
            {
                set_pixel(c, r, (Color) {0, 0, 10});
            }
        }
    }
    show_matrix(spi);
}


void draw_bubble(int l, int r, int t, int b, Color color)
{
    // Draw a bubble shape on the LED matrix
    set_pixel(l, t, color); // Top left
    set_pixel(r, t, color); // Top right
    set_pixel(l, b, color); // Bottom left
    set_pixel(r, b, color); // Bottom right
}
void bubble_level(float pitch, float roll, float lux)
{
    struct metal_spi *spi = metal_spi_get_device(2); // SPI1 controls the matrix
    if (spi == NULL) {
        return;
    }
    // Clear the LED matrix
    clear_matrix(spi);
    // clamp pitch to 15 deg and roll to 30
    pitch = -fmaxf(-15, fminf(pitch, 15));
    roll = -fmaxf(-30, fminf(roll, 30));

    // Calculate how level the device is
    float total_tilt = sqrt(pitch * pitch + roll * roll);

    // Map angles to bubble position
    // Center is at (LED_WIDTH/2, LED_HEIGHT/2)
    // ±30 degrees maps to ±2-3 pixels from center
    int bubble_center_x = LED_WIDTH / 2 + (int)(roll * (LED_WIDTH - 2) / 60.0f);
    int bubble_center_y = LED_HEIGHT / 2 + (int)(pitch * (LED_HEIGHT - 2) / 30.0f);

    int bubble_left = bubble_center_x - 1;
    int bubble_right = bubble_center_x;
    int bubble_top = bubble_center_y - 1;
    int bubble_bottom = bubble_center_y;

    if (bubble_left < 0)
        bubble_left = 0;
    if (bubble_right >= LED_WIDTH)
        bubble_right = LED_WIDTH - 1;
    if (bubble_top < 0)
        bubble_top = 0;
    if (bubble_bottom >= LED_HEIGHT)
        bubble_top = LED_HEIGHT - 1;

    if (total_tilt < 1.0f)
    {
        // fill green
        for (int x = 0; x < LED_WIDTH; x++)
        {
            for (int y = 0; y < LED_HEIGHT; y++) {
                set_pixel(x, y, (Color) {0, 10, 0}); // Fill the matrix with green
            }
        }
    }
    else if (total_tilt < 5.0f)
    {
        draw_bubble(bubble_left, bubble_right, bubble_top, bubble_bottom, (Color){0, 10, 0}); // Green
    }
    else if (total_tilt < 15.0f)
    {
        draw_bubble(bubble_left, bubble_right, bubble_top, bubble_bottom, (Color){10, 10, 0}); // Yellow
    }
    else if (total_tilt < 25.0f)
    {
        draw_bubble(bubble_left, bubble_right, bubble_top, bubble_bottom, (Color){10, 5, 0}); // Orange
    }
    else
    {
        draw_bubble(bubble_left, bubble_right, bubble_top, bubble_bottom, (Color){10, 0, 0}); // Red
    }

    show_matrix(spi); // Update the LED matrix display
}

void compass(int heading, Color color)
{
    // Clear the LED matrix
    struct metal_spi *spi = metal_spi_get_device(2); // SPI1 controls the matrix
    if (spi == NULL) {
        return;
    }
    clear_matrix(spi);

    // Normalize heading to 0-360 degrees
    if (heading < 0)
        heading += 360;
    if (heading >= 360)
        heading -= 360;

    // Calculate the index of the arrow based on the heading
    int arrow_index = ((heading % 360) + 22) / 45 % 8;
    // Draw the arrow for the current heading
    for (int i = 0; i < get_arrow_point_count(arrow_index); i++)
    {
        point_t point = get_arrow_point(arrow_index, i);
        set_pixel(point.x, point.y, color);
    }

    show_matrix(spi); // Update the LED matrix display
}

uint8_t brightness_from_lux(float lux)
{
    if (lux < MIN_LUX)
        lux = MIN_LUX;
    if (lux > MAX_LUX)
        lux = MAX_LUX;

    float norm = (lux - MIN_LUX) / (MAX_LUX - MIN_LUX);
    return MIN_BRIGHTNESS + (uint8_t)(norm * (MAX_BRIGHTNESS - MIN_BRIGHTNESS));
}

Color hsv_to_rgb(uint8_t h, uint8_t s, uint8_t v)
{
    uint8_t region = h / 43;
    uint8_t remainder = (h - (region * 43)) * 6;

    uint8_t p = (v * (255 - s)) >> 8;
    uint8_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    uint8_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
    case 0:
        return (Color){v, t, p};
    case 1:
        return (Color){q, v, p};
    case 2:
        return (Color){p, v, t};
    case 3:
        return (Color){p, q, v};
    case 4:
        return (Color){t, p, v};
    default:
        return (Color){v, p, q};
    }
}
