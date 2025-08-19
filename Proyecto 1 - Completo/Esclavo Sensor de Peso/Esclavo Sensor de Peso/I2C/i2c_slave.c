/*
 * i2c_slave.c
 *
 * Created: 19/08/2025 13:18:50
 *  Author: Chris Q
 */ 



#include "i2c_slave.h"

// Variables globales
volatile uint16_t tx_snap = 0;
volatile uint8_t tx_idx = 0;

void I2C_Slave_Init(uint8_t addr) {
	TWSR = 0x00;               // prescaler = 1
	TWAR = (addr << 1);        // dirección 7-bit
	TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWIE); // TWI + ACK + IRQ
}

// ISR: envía snapshot consistente (MSB, LSB)
ISR(TWI_vect) {
	uint8_t s = TWSR & 0xF8;

	switch (s) {
		// Receptor (SLA+W)
		case 0x60: case 0x68:
		case 0x80: case 0x90:
		case 0x88: case 0x98:
		case 0xA0:
		tx_idx = 0;
		TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWEA)|(1 << TWIE);
		break;

		// Transmisor (SLA+R)
		case 0xA8: // own SLA+R
		case 0xB0: // arb lost + SLA+R
		// Captura snapshot atómico
		tx_idx = 0;
		// Enviar MSB
		TWDR = (uint8_t)((tx_snap >> 8) & 0xFF);
		tx_idx = 1;
		TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWEA)|(1 << TWIE);
		break;

		case 0xB8: // dato transmitido; ACK
		if (tx_idx == 1) {
			// Enviar LSB del mismo snapshot
			TWDR = (uint8_t)(tx_snap & 0xFF);
			tx_idx = 2;
			} else {
			// Si pide más, repite LSB
			TWDR = (uint8_t)(tx_snap & 0xFF);
		}
		TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWEA)|(1 << TWIE);
		break;

		case 0xC0: // dato transmitido; NACK (fin)
		case 0xC8: // último dato transmitido; ACK
		default:
		tx_idx = 0;
		TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWEA)|(1 << TWIE);
		break;
	}
}