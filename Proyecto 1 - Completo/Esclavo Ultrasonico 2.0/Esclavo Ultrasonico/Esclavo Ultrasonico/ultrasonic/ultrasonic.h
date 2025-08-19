/*
 * ultrasonic.h
 *
 * Created: 16/08/2025 20:41:50
 *  Author: Chris Q
 */ 
#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <stdint.h>

/* Lectura por polling (no usa timers).
 * Devuelve cm en [2..400] o -1 si no hubo eco válido.
 */
int16_t ultrasonic_read_cm_blocking(void);

#endif

