#include "uart.h"
#include <stdio.h>
#include <metal/uart.h>
#include <stddef.h>

struct metal_uart *uart_device;

void init_uart(){
    uart_device = metal_uart_get_device(1); // Get UART to esp32
    if (uart_device == NULL) {
        printf("UART device not found!\n");
        return;
    }
    metal_uart_init(uart_device, 115200);
}

void print_esp(const char *packet, uint16_t length) {
    if (uart_device == NULL) {
        printf("UART device not initialized!\n");
        return;
    }
    for (size_t i = 0; i < length; i++) {
        metal_uart_putc(uart_device, packet[i]);
    }
}

uint8_t expected_command_length(uint8_t cmd){
    if (cmd == 0x01){ // Sensor Request Command
        return 2; // Start + checksum
    } else {
        return 255;
    }
}

void read_data(const SensorData *sensor_data) {
    if (uart_device == NULL) {
        printf("UART device not initialized!\n");
        return;
    }

    uint8_t buffer[64];
    uint8_t idx = 0;
    int c;

    while (1) {
        if (metal_uart_getc(uart_device, &c) == 0 && c != -1) {
            buffer[idx++] = (uint8_t)c;

            if (idx == expected_command_length(buffer[0])) {
                process_command(buffer, idx, sensor_data);
                idx = 0;
            }
        } else {
            // No new byte available — break to return to main loop
            break;
        }
    }
}

void process_command(const uint8_t buffer[64], uint8_t length, const SensorData *sensor_data) {
    if (length < 2){
        return;
    }
    uint8_t command = buffer[0];
    uint8_t checksum = buffer[length - 1];

    uint8_t calculated_checksum = 0;
    for (uint8_t i = 0; i < length - 1; i++) {
        calculated_checksum += buffer[i];
    }
    calculated_checksum &= 0xFF; // Ensure checksum is within byte range

    if (calculated_checksum != checksum) {
        // Checksum mismatch, handle error
        printf("Checksum mismatch: expected 0x%02X, got 0x%02X\n", calculated_checksum, checksum);
        return;
    }

    switch (command) {
        case 0x01: // Sensor Request Command
            send_sensor_data(sensor_data);
            break;
        default:
            // Unknown command, handle error or ignore
            break;
    }

}

void send_sensor_data(const SensorData *data){
    // Send data in compact byte packet to ESP32

    if (data == NULL) {
        return; // Handle null pointer
    }

    char packet[32];

    packet[0] = 0xAA; // Start byte

    // Accelerometer data
    packet[1] = data->accelerometer.x & 0xFF; // X low byte
    packet[2] = (data->accelerometer.x >> 8) & 0xFF; // X high byte
    packet[3] = data->accelerometer.y & 0xFF; // Y low byte
    packet[4] = (data->accelerometer.y >> 8) & 0xFF; // Y high byte
    packet[5] = data->accelerometer.z & 0xFF; // Z low byte
    packet[6] = (data->accelerometer.z >> 8) & 0xFF; // Z high byte

    // Temperature data
    union {
        float temp_float;
        uint8_t temp_bytes[4];
    } temp_union;
    temp_union.temp_float = data->temperature.temperature;

    packet[7] = temp_union.temp_bytes[0]; // Temperature byte 1
    packet[8] = temp_union.temp_bytes[1]; // Temperature byte 2
    packet[9] = temp_union.temp_bytes[2]; // Temperature byte 3
    packet[10] = temp_union.temp_bytes[3]; // Temperature byte 4

    // Magnetometer data
    packet[11] = data->magnetometer.x & 0xFF; // Magnetometer X low byte
    packet[12] = (data->magnetometer.x >> 8) & 0xFF; // Magnetometer X high byte
    packet[13] = data->magnetometer.y & 0xFF; // Magnetometer Y low byte
    packet[14] = (data->magnetometer.y >> 8) & 0xFF; // Magnetometer Y high byte
    packet[15] = data->magnetometer.z & 0xFF; // Magnetometer Z low byte
    packet[16] = (data->magnetometer.z >> 8) & 0xFF; // Magnetometer Z high byte

    // Magnetometer angles
    packet[17] = data->magnetometer.angle.heading & 0xFF; // Angle X low byte
    packet[18] = (data->magnetometer.angle.heading >> 8) & 0xFF; // Angle X high byte
    packet[19] = data->magnetometer.angle.pitch & 0xFF; // Angle Y low byte
    packet[20] = (data->magnetometer.angle.pitch >> 8) & 0xFF; // Angle Y high byte
    packet[21] = data->magnetometer.angle.roll & 0xFF; // Angle Z low byte
    packet[22] = (data->magnetometer.angle.roll >> 8) & 0xFF; // Angle Z high byte

    // Light sensor data
    packet[23] = data->light.ch0 & 0xFF; // Light CH0 low byte
    packet[24] = (data->light.ch0 >> 8) & 0xFF; // Light CH0 high byte
    packet[25] = data->light.ch1 & 0xFF; // Light CH1 low byte
    packet[26] = (data->light.ch1 >> 8) & 0xFF; // Light CH1 high byte

    // Light lux data
    union {
        float lux_float;
        uint8_t lux_bytes[4];
    } lux_union;
    lux_union.lux_float = data->light.lux;

    packet[27] = lux_union.lux_bytes[0]; // Lux byte 1
    packet[28] = lux_union.lux_bytes[1]; // Lux byte 2
    packet[29] = lux_union.lux_bytes[2]; // Lux byte 3
    packet[30] = lux_union.lux_bytes[3]; // Lux byte 4

    // Checksum
    uint8_t checksum = 0;
    for (int i = 0; i < 31; i++) {
        checksum += packet[i];
    }
    packet[31] = checksum & 0xFF; // Checksum byte
    print_esp(packet, 32);
}

