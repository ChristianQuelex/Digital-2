/*
 * uart.c
 *
 * Created: 1/08/2025 07:09:34
 *  Author: Chris Q
 */ 




#include "uart.h"
#include <stdlib.h> // Para itoa()

void uart_init(uint16_t baud_rate) {
	UBRR0H = (uint8_t)(baud_rate >> 8);
	UBRR0L = (uint8_t)baud_rate;
	UCSR0B = (1 << TXEN0); // Habilita solo TX
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 bits, 1 stop bit, sin paridad
}

void uart_print(const char *str) {
	while (*str) {
		while (!(UCSR0A & (1 << UDRE0))); // Espera buffer vacío
		UDR0 = *str++;
	}
}

void uart_print_number(uint16_t num) {
	char buffer[10];
	itoa(num, buffer, 10); // Convierte a string
	uart_print(buffer);
}