/*
 * ultrasonic.c
 *
 * Created: 16/08/2025 20:41:37
 * Author: Chris Q
 */
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

/* D5 (PD5) TRIG, D6 (PD6) ECHO */
#define ULTRA_TRIG_DDR   DDRD
#define ULTRA_TRIG_PORT  PORTD
#define ULTRA_TRIG_PIN   PD5

#define ULTRA_ECHO_DDR   DDRD
#define ULTRA_ECHO_PINR  PIND
#define ULTRA_ECHO_PIN   PD6

#define WAIT_RISING_TIMEOUT_US   25000UL
#define MAX_PULSE_US             25000UL
#define MIN_PULSE_US             150UL
#define MIN_CM                   2
#define MAX_CM                   400

int16_t ultrasonic_read_cm_blocking(void)
{
	ULTRA_TRIG_DDR  |=  (1 << ULTRA_TRIG_PIN);
	ULTRA_ECHO_DDR  &= ~(1 << ULTRA_ECHO_PIN);

	ULTRA_TRIG_PORT &= ~(1 << ULTRA_TRIG_PIN);
	_delay_us(5);

	ULTRA_TRIG_PORT |=  (1 << ULTRA_TRIG_PIN);
	_delay_us(10);
	ULTRA_TRIG_PORT &= ~(1 << ULTRA_TRIG_PIN);

	uint32_t guard = 0;
	while ((ULTRA_ECHO_PINR & (1 << ULTRA_ECHO_PIN)) == 0) {
		if (++guard >= WAIT_RISING_TIMEOUT_US) return -1;
		_delay_us(1);
	}

	uint32_t width_us = 0;
	while ((ULTRA_ECHO_PINR & (1 << ULTRA_ECHO_PIN)) != 0) {
		if (++width_us > MAX_PULSE_US) return -1;
		_delay_us(1);
	}

	if (width_us < MIN_PULSE_US) return -1;

	uint32_t cm = width_us / 58UL;
	if (cm < MIN_CM || cm > MAX_CM) return -1;

	return (int16_t)cm;
}
