/*
 * adc1.h
 *
 * Created: 1/08/2025 07:02:21
 *  Author: Chris Q
 */ 



#ifndef ADC_H
#define ADC_H

#include <avr/io.h>

void adc_init();
uint16_t adc_read(uint8_t channel);

#endif