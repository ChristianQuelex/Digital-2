/*
 * maestro_prueba_SPI.c
 *
 * Created: 31/07/2025 22:56:46
 * Author : Chris Q
 */ 
#include <avr/io.h>
#include <util/delay.h>
#include "spi/spi.h"
#include "uart/UART.h"

void init_leds() {
	DDRD |= 0b11111100; // PD2-PD7 como salidas
	DDRB |= 0b00000011; // PB0-PB1 como salidas
}

void show_number(uint8_t num) {
	PORTD = (PORTD & 0b00000011) | ((num & 0b00111111) << 2); // PD2-PD7 (bits 0-5)
	PORTB = (PORTB & 0b11111100) | ((num >> 6) & 0b00000011);  // PB0-PB1 (bits 6-7)
}

int main() {
	uart_init(103); // 9600 bauds @ 16MHz
	spi_master_init();
	init_leds();

	while (1) {
		uart_print("\nIngrese numero (0-255): ");
		uint8_t num = uart_receive_number();
		
		show_number(num); // Muestra en LEDs del maestro
		spi_master_transmit(num); // Envía al esclavo
		_delay_ms(500); // Espera para estabilizar

		// Opcional: Confirmación por UART
		uart_print("\nDato enviado: ");
		uart_print_number(num);
	}
}