/*
 * servo.c
 *
 * Created: 16/08/2025 20:41:08
 *  Author: Chris Q
 */ 
#include <avr/io.h>
#include "servo.h"

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/* Calibración (us) para mapear grados ? pulso */
static uint16_t g_min_us = 1000;  // ~0°
static uint16_t g_max_us = 2000;  // ~180°

static inline uint16_t us_to_ticks(uint16_t us)
{
	// 0.5 us por tick con prescaler=8 ? ticks = us * 2
	uint32_t t = (uint32_t)us * 2u;
	if (t < 1000u) t = 1000u;   // 0.5 ms mínimo seguro
	if (t > 5000u) t = 5000u;   // 2.5 ms máximo seguro
	return (uint16_t)t;
}

void servo_init(void)
{
	// PB1 (OC1A / D9) salida
	DDRB |= (1 << DDB1);

	// Timer1: Fast PWM, TOP = ICR1 (WGM13:0 = 14), no-inverting en OC1A
	TCCR1A = (1 << COM1A1) | (1 << WGM11);
	TCCR1B = (1 << WGM13)  | (1 << WGM12) | (1 << CS11);  // prescaler = 8

	// Periodo 20 ms (50 Hz)
	ICR1 = 39999;

	// Centro ~1500 us
	OCR1A = us_to_ticks(1500);
}

void servo_set_calibration(uint16_t min_us, uint16_t max_us)
{
	if (min_us < 500)  min_us = 500;
	if (max_us > 2500) max_us = 2500;
	if (min_us >= max_us) {
		min_us = 1000;
		max_us = 2000;
	}
	g_min_us = min_us;
	g_max_us = max_us;
}

void servo_set_ticks(uint16_t ticks)
{
	if (ticks < 1000)  ticks = 1000;
	if (ticks > 5000)  ticks = 5000;
	OCR1A = ticks;
}

void servo_set_us(uint16_t us)
{
	if (us < 500)   us = 500;
	if (us > 2500)  us = 2500;
	OCR1A = us_to_ticks(us);
}

void servo_set_deg(uint8_t deg)
{
	if (deg > 180) deg = 180;
	uint32_t span = (uint32_t)(g_max_us - g_min_us);
	uint16_t us = (uint16_t)(g_min_us + ((uint32_t)deg * span) / 180u);
	servo_set_us(us);
}
