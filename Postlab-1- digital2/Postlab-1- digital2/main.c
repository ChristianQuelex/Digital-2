/*
 * Postlab-1- digital2.c
 *
 * Created: 18/07/2025 07:47:34
 * Author : Chris Q
 */ 




#define F_CPU 16000000UL  // Added 'UL' for unsigned long

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <util/delay.h>
#include "Display/display.h"  // Ruta corregida

// Variables globales
volatile uint8_t start = 0;    // Added 'volatile' for ISR usage
volatile uint8_t counter1 = 0;
volatile uint8_t counter2 = 0;
volatile uint8_t shift1 = 0;
volatile uint8_t shift2 = 0;
volatile uint8_t block = 0;

void initPCINT0(void);
void initPCINT1(void);

int main(void) {
    cli();
    initDisplayPorts();  // Nombre corregido

    // Configuración de puertos (orden correcto: DDR -> PORT)
    DDRB = 0x0F;        // PB0-PB3 como salidas, PB4-PB7 como entradas
    PORTB |= (1 << PB4); // Pull-up en PB4

    DDRC = 0x0F;        // PC0-PC3 como salidas, PC4-PC7 como entradas
    PORTC |= (1 << PC4) | (1 << PC5); // Pull-ups en PC4 y PC5

    initPCINT0();
    initPCINT1();
    sei();

    while (1) {
        if (start == 1) { // Cuenta regresiva
            for (uint8_t i = 5; i > 0; i--) {
                display(i);
                _delay_ms(1000);
            }
            display(0);
            start = 2;
        }
    }
}

//************************************************************
//                 Interrupciones
//************************************************************

void initPCINT0(void) {
    PCICR |= (1 << PCIE0);
    PCMSK0 |= (1 << PCINT4);  // PB4
}

void initPCINT1(void) {
    PCICR |= (1 << PCIE1);
    PCMSK1 |= (1 << PCINT12) | (1 << PCINT13);  // PC4 y PC5
}

ISR(PCINT0_vect) {
    _delay_ms(20);  // Debounce básico
    if (!(PINB & (1 << PB4)) && (block == 0) && (start == 2)) {
        PORTC &= 0xF0;
        counter1++;
        shift1 = (counter1 - 1) % 4;  // Limita a 0-3
        PORTC |= (1 << shift1);

        if (shift1 == 3) {  // Si llega al final
            block = 1;
            display(1);
            PORTC |= 0x0F;
            PORTB &= 0xF0;
        }
    }
}

ISR(PCINT1_vect) {
    _delay_ms(20);  // Debounce básico
    
    // Botón de inicio (PC4)
    if (!(PINC & (1 << PC4)) && (start == 0)) {
        start = 1;
    }
    
    // Botón del jugador 2 (PC5)
    if (!(PINC & (1 << PC5)) && (block == 0) && (start == 2)) {
        PORTB &= 0xF0;
        counter2++;
        shift2 = (counter2 - 1) % 4;  // Limita a 0-3
        PORTB |= (1 << shift2);

        if (shift2 == 3) {  // Si llega al final
            block = 1;
            display(2);
            PORTB |= 0x0F;
            PORTC &= 0xF0;
        }
    }
}