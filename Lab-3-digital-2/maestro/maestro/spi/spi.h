/*
 * spi.h
 *
 * Created: 1/08/2025 07:10:12
 *  Author: Chris Q
 */ 

#ifndef SPI_H
#define SPI_H

#include <avr/io.h>

void spi_master_init();
void spi_master_transmit(uint8_t data);
uint8_t spi_master_receive();

#endif