/*
 * UART.c
 *
 * Created: 31/07/2025 23:00:32
 *  Author: Chris Q
 */ 

#include "uart.h"
#include <stdlib.h>

void uart_init(uint16_t baud_rate) {
	UBRR0H = (uint8_t)(baud_rate >> 8);
	UBRR0L = (uint8_t)baud_rate;
	UCSR0B = (1 << TXEN0) | (1 << RXEN0);  // Habilita TX y RX
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8 bits, sin paridad, 1 stop bit
}

void uart_print(char *str) {
	while (*str) {
		while (!(UCSR0A & (1 << UDRE0))); // Espera buffer vacío
		UDR0 = *str++;
	}
}

void uart_print_number(uint16_t num) {
	char buffer[10];
	itoa(num, buffer, 10); // Convierte número a string (base 10)
	uart_print(buffer);
}

uint8_t uart_receive_number() {
	char buffer[4];
	uint8_t i = 0;
	char c;

	while (i < sizeof(buffer) - 1) { // Evita overflow
		while (!(UCSR0A & (1 << RXC0))); // Espera dato
		c = UDR0;
		if (c == '\n' || c == '\r') break; // Fin de línea
		if (c >= '0' && c <= '9') buffer[i++] = c; // Solo dígitos
	}
	buffer[i] = '\0'; // Termina el string
	return (uint8_t)atoi(buffer);
	
	uint16_t num = atoi(buffer);
	return (num <= 255) ? (uint8_t)num : 0;
}