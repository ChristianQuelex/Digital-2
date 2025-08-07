/*
 * ADC.c
 *
 * Created: 7/08/2025 06:31:44
 *  Author: Chris Q
 */ 


// ADC.c
#include "ADC.h"
#include <avr/io.h>

// Configuración del ADC (igual a tu versión original)
void ADC_init(void) {
	ADMUX |= (1<<REFS0);    // Seleccionar el voltaje de referencia
	ADMUX &= ~(1<<REFS1);
	ADMUX |= (1<<ADLAR);    // Se define que se trabajara con 8 bits (ADCH)
	
	// Configuracion Prescaler de 128 --> 16M/128 = 125KHz
	ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
	ADCSRA |= (1<<ADEN);    // Se enciende el ADC
	
	// Entradas para los potenciometros
	DIDR0 |= (1 << ADC0D);  // Deshabilitar la entrada digital PC0
}

// Función para leer el ADC (igual a tu versión original)
uint16_t ADC_read(uint8_t canal) {
	ADMUX = (ADMUX & 0xF0) | (canal & 0x0F);  // Seleccionar el canal
	ADCSRA |= (1<<ADSC);                      // Iniciar conversión
	while(ADCSRA & (1<<ADSC));                // Esperar fin de conversión
	return ADCH;                              // Devolver los 8 bits más significativos
}