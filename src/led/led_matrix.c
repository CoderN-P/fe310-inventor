#include "led_matrix.h"
#include "../sensors/accelerometer.h"
#include "../utils/delay.h"
#include "./compass.h"
#include <stdio.h>
#include <math.h>
#include <metal/cpu.h>
#include <stdint.h>
#include <metal/gpio.h>
#include "fluid_sim.h"

Color led_matrix[LED_WIDTH * LED_HEIGHT];
uint64_t last_led_update_time;
struct metal_gpio* gpio_device;

void disable_interrupts()
{
    asm volatile("csrrc x0, mstatus, %0" ::"r"(0x8)); // Clear MIE bit in mstatus
}

void enable_interrupts()
{
    asm volatile("csrrs x0, mstatus, %0" ::"r"(0x8)); // Set MIE bit in mstatus
}

void gpio_direct_init(void)
{
    gpio_device = metal_gpio_get_device(0);
     if (!gpio_device) {
        printf("Failed to get GPIO device.\n");
        return;
    }
     metal_gpio_enable_output(gpio_device, LED_MATRIX_GPIO);
}

void send_bit(uint8_t bit)
{
    if (bit)
    {
        gpio_high(); // Send "1" bit
    }
    else
    {
        gpio_low(); // Send "0" bit
    }
}

void send_byte(uint8_t byte)
{
    for (int i = 0; i < 8; i++)
    {
        send_bit((byte >> (7 - i)) & 1);
    }
}

void send_color(uint8_t r, uint8_t g, uint8_t b)
{
    // Send color data in GRB format
    send_byte(g);
    send_byte(r);
    send_byte(b);
}

void clear()
{
    // Clear the LED matrix by sending black color to all LEDs
    for (int i = 0; i < NUM_LEDS; i++)
    {
        send_color(0, 0, 0);
    }
    delay_us(500); // Wait for 500us to latch the data
}

void show()
{
    // Send color data for each LED in the matrix
    for (int i = 0; i < LED_HEIGHT * LED_WIDTH; i++)
    {
        send_color(led_matrix[i].r, led_matrix[i].g, led_matrix[i].b);
    }
    delay_us(500); // guarantee reset from whatever state we were in
}

void clear_led_matrix()
{
    // Clear the LED matrix by setting all pixels to black
    for (int i = 0; i < LED_HEIGHT * LED_WIDTH; i++)
    {
        led_matrix[i] = (Color){0, 0, 0}; // Set each pixel to black
    }
}

void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
    if (x < 0 || x >= LED_WIDTH || y < 0 || y >= LED_HEIGHT)
    {
        return; // Out of bounds
    }
    led_matrix[y * LED_WIDTH + LED_WIDTH - 1 - x] = (Color){r, g, b}; // Set pixel color
}

void draw_bubble(int l, int r, int t, int b, Color color)
{
    // Draw a bubble shape on the LED matrix
    set_pixel(l, t, color.r, color.g, color.b); // Top left
    set_pixel(r, t, color.r, color.g, color.b); // Top right
    set_pixel(l, b, color.r, color.g, color.b); // Bottom left
    set_pixel(r, b, color.r, color.g, color.b); // Bottom right
}
void bubble_level(float pitch_raw, float roll_raw, float lux)
{
    // Calculate how level the device is
    clear_led_matrix();
    // clamp pitch and roll to 30 deg magnitude

    float pitch = fmaxf(fminf(pitch_raw, 30), -30);
    float roll = fmaxf(fminf(roll_raw, 30), -30);
    float total_tilt = sqrt(pitch * pitch + roll * roll);

    // Map angles to bubble position
    // Center is at (LED_WIDTH/2, LED_HEIGHT/2)
    // ±30 degrees maps to ±2-3 pixels from center
    int bubble_center_x = LED_WIDTH / 2 + (int)(roll * (LED_WIDTH - 2) / 60.0f);
    int bubble_center_y = LED_HEIGHT / 2 + (int)(pitch * (LED_HEIGHT - 2) / 30.0f);

    if (bubble_center_x < 1)
        bubble_center_x = 1;
    if (bubble_center_x >= LED_WIDTH)
        bubble_center_x = LED_WIDTH - 1;
    if (bubble_center_y < 1)
        bubble_center_y = 1;
    if (bubble_center_y >= LED_HEIGHT)
        bubble_center_y = LED_HEIGHT - 1;

    int bubble_left = bubble_center_x - 1;
    int bubble_right = bubble_center_x;
    int bubble_top = bubble_center_y - 1;
    int bubble_bottom = bubble_center_y;

    float brightness = brightness_from_lux(lux);

    if (total_tilt < 1.0f)
    {
        // fill green
        for (int i = 0; i < LED_HEIGHT * LED_WIDTH; i++)
        {
            led_matrix[i] = (Color){0, brightness, 0}; // Fill the matrix with green
        }
    }
    else if (total_tilt < 5.0f)
    {
        draw_bubble(bubble_left, bubble_right, bubble_top, bubble_bottom, (Color){0, brightness, 0}); // Green
    }
    else if (total_tilt < 15.0f)
    {
        draw_bubble(bubble_left, bubble_right, bubble_top, bubble_bottom, (Color){brightness, brightness, 0}); // Yellow
    }
    else if (total_tilt < 25.0f)
    {
        draw_bubble(bubble_left, bubble_right, bubble_top, bubble_bottom, (Color){brightness, brightness / 2, 0}); // Orange
    }
    else
    {
        draw_bubble(bubble_left, bubble_right, bubble_top, bubble_bottom, (Color){brightness, 0, 0}); // Red
    }

    show(); // Update the LED matrix display
}

void compass(int heading, Color color, float lux)
{
    // Clear the LED matrix
    clear_led_matrix();
    // Normalize heading to 0-360 degrees
    if (heading < 0)
        heading += 360;
    if (heading >= 360)
        heading -= 360;

    float brightness = brightness_from_lux(lux);

    // Calculate the index of the arrow based on the heading
    int arrow_index = ((heading % 360) + 22) / 45 % 8;
    // Draw the arrow for the current heading
    for (int i = 0; i < get_arrow_point_count(arrow_index); i++)
    {
        point_t point = get_arrow_point(arrow_index, i);
        set_pixel(point.x, point.y, (color.r * brightness) / 255, (color.g * brightness) / 255, (color.b * brightness) / 255);
    }

    show(); // Update the LED matrix display
}

void test(float lux)
{
    clear_led_matrix();

    float brightness = brightness_from_lux(lux);
    for (int r = 0; r < LED_HEIGHT; r++)
    {
        for (int c = 0; c < LED_WIDTH; c++)
        {
            if ((r + c) % 2 == 0)
            {
                set_pixel(c, r, 0, brightness, 0);
            }
            else
            {
                set_pixel(c, r, 0, 0, brightness);
            }
        }
    }
    show();
}

void fluid_sim(float pitch, float roll, float lux)
{
    // convert pitch and roll to radians
    metal_gpio_set_pin(gpio_device, LED_MATRIX_GPIO, 0);
    float pitch_rad = pitch * M_PI / 180.0f;
    float roll_rad = roll * M_PI / 180.0f;
    update(pitch_rad, roll_rad);

    float brightness = brightness_from_lux(lux);
    for (int x = 1; x < WIDTH - 1; x++)
    {
        for (int y = 1; y < HEIGHT - 1; y++)
        {
            if (cellType[x][y] == WATER)
            {
                float density = particleDensity[x][y];
                float t = 0;

                if (particleRestDensity > 0.0f)
                {
                    t = density / particleRestDensity;
                }

                uint8_t pixel_brightness = (uint8_t)clamp(t * brightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
                set_pixel(x - 1, y - 1, 0, 0, pixel_brightness);
            }
            else
            {
                set_pixel(x - 1, y - 1, 0, 0, 0);
            }
        }
    }

    show();
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
