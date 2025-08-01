/*
 * spi.h
 *
 * Created: 1/08/2025 07:01:46
 *  Author: Chris Q
 */ 




#ifndef SPI_H
#define SPI_H

#include <avr/io.h>

void spi_slave_init();
uint8_t spi_slave_receive();
void spi_slave_transmit(uint8_t data);

#endif