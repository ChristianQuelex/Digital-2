/*
 * servo.h
 *
 * Created: 16/08/2025 20:41:20
 *  Author: Chris Q
 */ 




#ifndef SERVO_H
#define SERVO_H

#include <avr/io.h>

// Constantes de posición del servo
#define SERVO_MIN 2000   // 1 ms (2000 ticks con prescaler=8)
#define SERVO_MAX 4000   // 2 ms (4000 ticks con prescaler=8)

void servo_init(void);
void servo_set(uint16_t position);

#endif
