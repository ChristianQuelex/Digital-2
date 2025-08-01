/*
 * CFile1.c
 *
 * Created: 31/07/2025 23:13:24
 *  Author: Chris Q
 */ 


#include "spi.h"

void spi_slave_init() {
	DDRB |= (1 << DDB4); // MISO (D12) como salida
	SPCR = (1 << SPE);   // Habilita SPI (modo esclavo)
}

uint8_t spi_slave_receive() {
	while (!(SPSR & (1 << SPIF))); // Espera dato del maestro
	return SPDR;
}

void spi_slave_transmit(uint8_t data) {
	SPDR = data;                   // Carga dato a enviar
	while (!(SPSR & (1 << SPIF))); // Espera fin de transmisión
}