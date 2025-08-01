/*
 * spi.c
 *
 * Created: 1/08/2025 07:10:25
 *  Author: Chris Q
 */ 

#include "spi.h"

void spi_master_init() {
	// Configura MOSI (PB3), SCK (PB5), SS (PB2) como salidas
	DDRB |= (1 << DDB3) | (1 << DDB5) | (1 << DDB2);
	// Habilita SPI, Modo Maestro, Reloj = fosc/16
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
}

void spi_master_transmit(uint8_t data) {
	PORTB &= ~(1 << PORTB2); // SS en bajo (activa esclavo)
	SPDR = data; // Carga el dato
	while (!(SPSR & (1 << SPIF))); // Espera fin de transmisión
	PORTB |= (1 << PORTB2); // SS en alto (desactiva esclavo)
}

uint8_t spi_master_receive() {
	while (!(SPSR & (1 << SPIF))); // Espera dato recibido
	return SPDR;
}