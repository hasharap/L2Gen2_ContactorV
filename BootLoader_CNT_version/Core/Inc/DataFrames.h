#ifndef DATAFRAMES_H_
#define DATAFRAMES_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <OTA.h>

#define OTA_SOF  0xA1                           // Start of Frame
#define OTA_EOF  0x0A                           // End of Frame
#define OTA_ACK  0x01                           // ACK
#define OTA_NACK 0x02                           // NACK

#define OTA_HANDSHAKE  0x23
#define OTA_START      0x01
#define OTA_APP_SIZE   0x02
#define OTA_DATA       0x03
#define OTA_END        0x04
#define OTA_RESPONSE   0x05

typedef enum {
	OTA_STATE_IDLE = 0,
	OTA_STATE_HANDSHAKE = 1,
	OTA_STATE_START = 2,
	OTA_STATE_HEADER = 3,
	OTA_STATE_DATA = 4,
	OTA_STATE_END = 5,
} OTA_STATE_;

/*
 * OTA Data format
 *
 * _____________
 * |     |        |     |        |     |
 * | SOF |  Data  | crc |  Star  | nl  |
 * |__|__|_|__|__|
 *   1B      20B     2B    1B      1B
 */

typedef struct {
	uint8_t sof;
	uint8_t data[15];
	uint16_t crc;
	uint8_t star;
	uint8_t newline;
} __attribute__((packed)) OTA_SOM_;

/*
 * OTA Command format
 *
 * ________________________________________
 * |     | Packet |     |     |     |     |
 * | SOF | Type   | Len | CMD | CRC | EOF |
 * |_____|________|_____|_____|_____|_____|
 *   1B      1B     2B    1B     4B    1B
 */

typedef struct {
	uint8_t sof;
	uint8_t packet_type;
	uint16_t data_len;
	uint16_t cmd;
	uint16_t crc;
	uint8_t eof;
} __attribute__((packed)) OTA_COMMAND_;

/*
 * OTA Header format
 *
 * __________________________________________
 * |     | Packet |     | Header |     |     |
 * | SOF | Type   | Len |  Data  | CRC | EOF |
 * |_____|________|_____|________|_____|_____|
 *   1B      1B     2B     16B     4B    1B
 */

typedef struct {
	uint8_t sof;
	uint8_t packet_type;
	uint16_t data_len;
	uint16_t data;
	uint16_t crc;
	uint8_t eof;
} __attribute__((packed)) OTA_HEADER_;

/*
 * OTA Data format
 *
 * __________________________________________
 * |     | Packet |     |        |     |     |
 * | SOF | Type   | Len |  Data  | CRC | EOF |
 * |_____|________|_____|________|_____|_____|
 *   1B      1B     2B    nBytes   4B    1B
 */

typedef struct {
	uint8_t sof;
	uint8_t packet_type;
	uint16_t data_len;
	uint8_t data[16];
	uint16_t crc;
	uint8_t eof;
} __attribute__((packed)) OTA_DATA_;

/*
 * OTA Response format
 *
 * __________________________________________
 * |     | Packet |     |        |     |     |
 * | SOF | Type   | Len | Status | CRC | EOF |
 * |_____|________|_____|________|_____|_____|
 *   1B      1B     2B      1B     4B    1B
 */

typedef struct {
	uint8_t sof;
	uint8_t packet_type;
	uint16_t data_len;
	uint8_t status;
	uint16_t crc;
	uint8_t eof;
} __attribute__((packed)) OTA_RESP_;

#endif

uint16_t calc_CRC(uint8_t data[], uint8_t len) {
	uint16_t crc = 0xFFFF;

	for (int pos = (len - 1); pos >= 0; pos--) {
		crc ^= (uint16_t) data[pos];     // XOR byte into least sig. byte of crc

		for (int i = 8; i != 0; i--) {    // Loop over each bit
			if ((crc & 0x0001) != 0) {      // If the LSB is set
				crc >>= 1;                    // Shift right and XOR 0xA001
				crc ^= crc_val;
			} else
				// Else LSB is not set
				crc >>= 1;                    // Just shift right
		}
	}
	// Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
	return crc;
}

void reverseArray(uint8_t arr[], int size) {
	int start = 0;
	int end = size - 1;

	while (start < end) {
		// Swap elements at start and end indices
		uint8_t temp = arr[start];
		arr[start] = arr[end];
		arr[end] = temp;

		// Move indices towards the center
		start++;
		end--;
	}
}

