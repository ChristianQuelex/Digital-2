/*
 * spi.c
 *
 * Created: 1/08/2025 07:01:32
 *  Author: Chris Q
 */ 


#include "spi.h"

void spi_slave_init() {
	DDRB |= (1 << PORTB4); // MISO (PB4) como salida
	SPCR = (1 << SPE);  // Habilita SPI (modo esclavo)
}

uint8_t spi_slave_receive() {
	while (!(SPSR & (1 << SPIF))); // Espera dato del maestro
	return SPDR;
}

void spi_slave_transmit(uint8_t data) {
	SPDR = data; // Carga el dato a enviar
	while (!(SPSR & (1 << SPIF))); // Espera fin de transmisión
}