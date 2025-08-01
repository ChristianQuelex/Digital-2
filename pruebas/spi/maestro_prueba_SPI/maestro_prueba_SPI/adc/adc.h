/*
 * ADC.h
 *
 * Created: 31/07/2025 22:59:32
 *  Author: Chris Q
 */ 

#ifndef ADC_H
#define ADC_H

#include <avr/io.h>

void adc_init();
uint16_t adc_read(uint8_t channel);

#endif