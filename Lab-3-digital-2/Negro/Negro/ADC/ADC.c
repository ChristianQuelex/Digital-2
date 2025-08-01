/*
 * ADC.c
 *
 * Created: 25/07/2025 05:52:22
 *  Author: Chris Q
 */ 


#include "ADC.h"

void ADC_init(void) {
	ADMUX = (1 << REFS0);               // AVcc como referencia
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Prescaler 128
	DIDR0 = (1 << ADC2D) | (1 << ADC3D); // Deshabilitar entradas digitales
}

uint16_t ADC_read(uint8_t channel) {
	ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
	ADCSRA |= (1 << ADSC);
	while(ADCSRA & (1 << ADSC));
	uint8_t low = ADCL;  // Leer ADCL primero!
	uint8_t high = ADCH;
	return (high << 8) | low;
}