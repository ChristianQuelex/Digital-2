/*
 * CFile1.c
 *
 * Created: 31/07/2025 23:00:57
 *  Author: Chris Q
 */ 


#include "spi.h"

void spi_master_init() {
	DDRB |= (1 << DDB3) | (1 << DDB5) | (1 << DDB2); // MOSI (D11), SCK (D13), SS (D10) como salidas
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);   // SPI maestro, fosc/16
}

void spi_master_transmit(uint8_t data) {
	PORTB &= ~(1 << PORTB2); // SS (D10) en bajo
	SPDR = data;
	while (!(SPSR & (1 << SPIF))); // Espera fin de transmisión
	PORTB |= (1 << PORTB2); // SS (D10) en alto
}

uint8_t spi_master_receive() {
	while (!(SPSR & (1 << SPIF))); // Espera dato del esclavo
	return SPDR;
}