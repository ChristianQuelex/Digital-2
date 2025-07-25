/*
 * CFile1.c
 *
 * Created: 11/07/2025 02:46:17
 *  Author: Chris Q
 */ 


#include <avr/io.h>
#include "Display.h"

// Mapeo para display de CÁTODO COMÚN (cambiar si es ánodo común)
const uint8_t digitos[10] = {
	0x3F, // 0
	0x06, // 1
	0x5B, // 2
	0x4F, // 3
	0x66, // 4
	0x6D, // 5
	0x7D, // 6
	0x07, // 7
	0x7F, // 8
	0x6F  // 9
};

void initDislayPorts(void) {
	DDRD = 0xFF;    // PORTD como salida
	PORTD = 0x00;   // Display apagado inicialmente
}

void display(uint8_t dig) {
	if(dig < 10) {
		PORTD = digitos[dig];
		} else {
		PORTD = 0x00; // Apagar display si no es válido
	}
}