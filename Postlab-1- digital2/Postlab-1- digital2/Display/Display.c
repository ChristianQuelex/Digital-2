/*
 * CFile1.c
 *
 * Created: 18/07/2025 07:54:13
 *  Author: Chris Q
 */ 


#include "display.h"  // Inclusión corregida
#include <avr/io.h>

void initDisplayPorts(void) {  // Nombre corregido
    DDRD = 0xFF;    // PORTD como salida
    PORTD = 0x00;   // Apagar todos los segmentos
    UCSR0B = 0;     // Deshabilitar UART para usar los pines normales
}

void display(uint8_t digit) {
    const uint8_t segments[] = {
        0x3F, 0x06, 0x5B, 0x4F,  // 0-3
        0x66, 0x6D, 0x7D, 0x07,  // 4-7
        0x7F, 0x6F, 0x77, 0x7C,  // 8-11
        0x39, 0x5E, 0x79, 0x71   // 12-15
    };
    
    if (digit < 16) {
        PORTD = segments[digit];
    }
}