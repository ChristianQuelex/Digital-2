/*
 * hx711.h
 *
 * Created: 19/08/2025 13:18:23
 *  Author: Chris Q
 */ 

#ifndef HX711_H
#define HX711_H

#include <avr/io.h>
#include <util/delay.h>

// Configuración de pines
#define HX711_DT   PD3
#define HX711_SCK  PD2

// Prototipos de funciones
void HX711_init(void);
uint8_t HX711_is_ready(void);
long HX711_read_once(void);
long HX711_read_avg(uint8_t n);
long tare_hx711(uint16_t n);

#endif