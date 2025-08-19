/*
 * servo.h
 *
 * Created: 16/08/2025 20:41:20
 *  Author: Chris Q
 */ 

#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>

/* Servo en OC1A (PB1 / D9) usando Timer1 a 50 Hz.
 * F_CPU = 16 MHz, prescaler = 8 ? 0.5 us/tick.
 * Periodo 20 ms ? 40000 ticks ? ICR1 = 39999.
 */

void servo_init(void);

/* Ajusta el recorrido útil del servo en microsegundos (500..2500 us).
 * Útil si tu servo solo recorre ~60° y quieres ampliar a ~90–120°:
 *   servo_set_calibration(900, 2100);
 */
void servo_set_calibration(uint16_t min_us, uint16_t max_us);

void servo_set_ticks(uint16_t ticks);  // 0.5 us por tick
void servo_set_us(uint16_t us);        // 500..2500 us (clipping)
void servo_set_deg(uint8_t deg);       // 0..180°, mapeado a [min_us..max_us]

#endif
