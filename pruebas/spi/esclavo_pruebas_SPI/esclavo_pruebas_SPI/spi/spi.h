/*
 * IncFile1.h
 *
 * Created: 31/07/2025 23:13:31
 *  Author: Chris Q
 */ 


#ifndef SPI_H
#define SPI_H

#include <avr/io.h>

void spi_slave_init();
uint8_t spi_slave_receive();
void spi_slave_transmit(uint8_t data);

#endif