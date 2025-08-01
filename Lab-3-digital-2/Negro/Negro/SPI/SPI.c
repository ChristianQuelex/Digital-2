/*
 * SPI.c
 *
 * Created: 25/07/2025 05:53:05
 *  Author: Chris Q
 */ 
#include "SPI.h"

void SPI_init(uint8_t mode, uint8_t data_order, uint8_t clock_polarity, uint8_t clock_phase) {
	if(mode == SPI_MASTER) {
		DDRB |= (1 << DDB3) | (1 << DDB5); // MOSI y SCK como salidas
		SPCR = (1 << SPE) | (1 << MSTR);
		} else {
		DDRB |= (1 << DDB4); // MISO como salida
		SPCR = (1 << SPE);
	}
	
	if(data_order == SPI_LSB_FIRST) SPCR |= (1 << DORD);
	if(clock_polarity == SPI_CLOCK_IDLE_HIGH) SPCR |= (1 << CPOL);
	if(clock_phase == SPI_SAMPLE_ON_TRAILING) SPCR |= (1 << CPHA);
}

void SPI_enable_interrupt(void) {
	SPCR |= (1 << SPIE);
}

uint8_t SPI_receive(void) {
	return SPDR;
}

void SPI_transmit(uint8_t data) {
	SPDR = data;
}