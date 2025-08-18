/*
 * servo.c
 *
 * Created: 16/08/2025 20:41:08
 *  Author: Chris Q
 */ 



#include "servo.h"

void servo_init(void) {
	DDRB |= (1 << PB1); // OC1A como salida
	TCCR1A = (1 << COM1A1) | (1 << WGM11);
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11); // Fast PWM, prescaler=8
	ICR1 = 40000;   // 20 ms (50 Hz)
	OCR1A = SERVO_MIN;
}

void servo_set(uint16_t position) {
	if (position < SERVO_MIN) position = SERVO_MIN;
	if (position > SERVO_MAX) position = SERVO_MAX;
	OCR1A = position;
}
