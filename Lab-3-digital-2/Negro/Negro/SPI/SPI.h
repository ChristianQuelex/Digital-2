/*
 * SPI.h
 *
 * Created: 25/07/2025 05:52:48
 *  Author: Chris Q
 */ 

#ifndef SPI_H
#define SPI_H

#include <avr/io.h>
#include <stdint.h>

// Modos de operación
#define SPI_SLAVE 0
#define SPI_MASTER 1

// Orden de bits
#define SPI_MSB_FIRST 0
#define SPI_LSB_FIRST 1

// Polaridad de reloj
#define SPI_CLOCK_IDLE_LOW 0
#define SPI_CLOCK_IDLE_HIGH 1

// Fase de reloj
#define SPI_SAMPLE_ON_LEADING 0
#define SPI_SAMPLE_ON_TRAILING 1

void SPI_init(uint8_t mode, uint8_t data_order, uint8_t clock_polarity, uint8_t clock_phase);
void SPI_enable_interrupt(void);
uint8_t SPI_receive(void);
void SPI_transmit(uint8_t data);

#endif