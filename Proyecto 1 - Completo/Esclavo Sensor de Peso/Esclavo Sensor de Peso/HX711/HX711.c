/*
 * HX711.c
 *
 * Created: 19/08/2025 13:17:54
 *  Author: Chris Q
 */ 



#include "hx711.h"

void HX711_init(void) {
	DDRD &= ~(1 << HX711_DT);   // DT entrada
	DDRD |=  (1 << HX711_SCK);  // SCK salida
	PORTD &= ~(1 << HX711_SCK); // SCK en bajo
}

uint8_t HX711_is_ready(void) {
	return (PIND & (1 << HX711_DT)) == 0;
}

long HX711_read_once(void) {
	long v = 0;

	// Espera BLOQUEANTE a listo
	while (!HX711_is_ready());

	// 24 bits MSB-first
	PORTD &= ~(1 << HX711_SCK);
	_delay_us(1);
	for (uint8_t i = 0; i < 24; i++) {
		PORTD |=  (1 << HX711_SCK);
		_delay_us(1);
		v <<= 1;
		if (PIND & (1 << HX711_DT)) v++;
		PORTD &= ~(1 << HX711_SCK);
		_delay_us(1);
	}

	// Pulso extra (ganancia 128, canal A)
	PORTD |=  (1 << HX711_SCK);
	_delay_us(1);
	PORTD &= ~(1 << HX711_SCK);
	_delay_us(1);

	// Extensión de signo (24-bit signed -> 32-bit)
	if (v & 0x800000L) v |= 0xFF000000L;

	return v;
}

long HX711_read_avg(uint8_t n) {
	long acc = 0;
	for (uint8_t i = 0; i < n; i++) acc += HX711_read_once();
	return acc / (long)n;
}

long tare_hx711(uint16_t n) {
	// descarta algunas para "calentar"
	for (uint8_t i = 0; i < 8; i++) (void)HX711_read_once();
	long acc = 0;
	for (uint16_t i = 0; i < n; i++) acc += HX711_read_avg(4); // AVG_SAMPLES hardcodeado
	return acc / (long)n;
}