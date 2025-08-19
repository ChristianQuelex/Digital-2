/*
 * ultrasonic.c
 *
 * Created: 16/08/2025 20:41:37
 *  Author: Chris Q
 */ 





#include "ultrasonic.h"

void ultrasonic_init(void) {
	DDRD |= (1 << TRIG_PIN);   // Trigger salida
	DDRD &= ~(1 << ECHO_PIN);  // Echo entrada
}

uint16_t measure_distance(void) {
	// Pulso de trigger
	PORTD &= ~(1 << TRIG_PIN);
	_delay_us(2);
	PORTD |= (1 << TRIG_PIN);
	_delay_us(10);
	PORTD &= ~(1 << TRIG_PIN);

	// Esperar inicio de eco
	while(!(PIND & (1 << ECHO_PIN)));
	TCNT1 = 0;
	while(PIND & (1 << ECHO_PIN));
	uint16_t duration = TCNT1;

	// Convertir ticks (0.5 us cada uno con prescaler=8 y F_CPU=16 MHz) a distancia
	return (duration * 0.5 * 0.0343) / 2;
}
