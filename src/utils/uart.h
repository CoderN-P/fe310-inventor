#ifndef UART_H
#define UART_H

#include "../sensors/sensors.h"
#include <stddef.h>

void init_uart(void);
void print_esp(const char *packet, uint16_t length);
void send_sensor_data(const SensorData *data);
void process_command(const uint8_t *buffer, uint8_t length, const SensorData *sensor_data);
void read_data(const SensorData *sensor_data);
uint8_t expected_command_length(uint8_t cmd);

/*
 * Sensor Packet
 * This structure is used to send sensor data in a compact byte packet format. (LITTLE ENDIAN)
 *
 * The packet format is as follows:
 * - Byte 1: 0xAA (Start byte)
 * - Bytes 2-3: Accelerometer X (2 bytes, signed 16-bit)
 * - Bytes 4-5: Accelerometer Y (2 bytes, signed 16-bit)
 * - Bytes 6-7: Accelerometer Z (2 bytes, signed 16-bit)
 * - Bytes 8-11: Temperature (4 bytes, float)
 * - Bytes 12-13: Magnetometer X (2 bytes, signed 16-bit)
 * - Bytes 14-15: Magnetometer Y (2 bytes, signed 16-bit)
 * - Bytes 16-17: Magnetometer Z (2 bytes, signed 16-bit)
 * - Bytes 18-19: Magnetometer Angle X (2 bytes, signed 16-bit)
 * - Bytes 20-21: Magnetometer Angle Y (2 bytes, signed 16-bit)
 * - Bytes 22-23: Magnetometer Angle Z (2 bytes, signed 16-bit)
 * - Bytes 24-25: Light Sensor CH0 (2 bytes, unsigned 16-bit)
 * - Bytes 26-27: Light Sensor CH1 (2 bytes, unsigned 16-bit)
 * - Bytes 28-31: Light Sensor Lux (4 bytes, float)
 * - Byte 32: Checksum (1 byte, sum of all previous bytes modulo 256)
 *
 */

#endif