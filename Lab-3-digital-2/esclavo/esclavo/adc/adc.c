/*
 * adc.c
 *
 * Created: 1/08/2025 07:02:39
 *  Author: Chris Q
 */ 



#include "adc.h"

void adc_init() {
	ADMUX = (1 << REFS0); // AVCC como referencia (5V)
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // ADC enable, prescaler 128
}

uint16_t adc_read(uint8_t channel) {
	ADMUX = (ADMUX & 0xF0) | (channel & 0x0F); // Selecciona canal
	ADCSRA |= (1 << ADSC); // Inicia conversión
	while (ADCSRA & (1 << ADSC)); // Espera fin de conversión
	return ADC; // Retorna valor (0-1023)
}