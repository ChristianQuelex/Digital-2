/*
 * ultrasonic.h
 *
 * Created: 16/08/2025 20:41:50
 *  Author: Chris Q
 */ 





#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#define TRIG_PIN PD5
#define ECHO_PIN PD6

void ultrasonic_init(void);
uint16_t measure_distance(void);

#endif
