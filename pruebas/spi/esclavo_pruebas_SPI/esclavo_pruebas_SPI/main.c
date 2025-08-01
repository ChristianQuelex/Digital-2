/*
 * esclavo_pruebas_SPI.c
 *
 * Created: 31/07/2025 22:57:44
 * Author : Chris Q
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include "spi/spi.h"

void init_leds() {
	DDRD |= 0b11111100; // PD2-PD7 como salidas
	DDRB |= 0b00000011; // PB0-PB1 como salidas
}

void show_number(uint8_t num) {
	PORTD = (PORTD & 0b00000011) | ((num & 0b00111111) << 2); // PD2-PD7
	PORTB = (PORTB & 0b11111100) | ((num >> 6) & 0b00000011); // PB0-PB1
}

int main() {
	spi_slave_init();
	init_leds();
	uint8_t last_num = 0; // Almacena el último número recibido

	while (1) {
		uint8_t num = spi_slave_receive();
		if (num != last_num) { // Solo actualiza si cambió
			show_number(num);
			last_num = num;
		}
	}
}