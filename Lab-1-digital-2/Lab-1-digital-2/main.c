/*
 * Lab-1-digital-2.c
 *
 * Created: 11/07/2025 02:45:40
 * Author : Chris Q
 */ 


#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "Display/Display.h"

// Variables globales
volatile uint8_t estado = 0;      // 0=inactivo 1=conteo 2=carrera
volatile uint8_t jugador1 = 0;    // Contador jugador 1 (PB0-PB3)
volatile uint8_t jugador2 = 0;    // Contador jugador 2 (PC0-PC3)

void configurarInterrupciones(void);
void actualizarContadores(void);

int main(void) {
	// 1. Configuración inicial
	initDislayPorts();
	DDRB = 0x0F;    // PB0-PB3 como salidas
	DDRC = 0x0F;    // PC0-PC3 como salidas
	
	// Habilitar pull-ups para botones
	PORTB |= (1 << PB4);
	PORTC |= (1 << PC4) | (1 << PC5);
	
	// 2. Configurar interrupciones
	configurarInterrupciones();
	sei();
	
	// 3. Bucle principal
	while(1) {
		switch(estado) {
			case 1: // Conteo regresivo
			for(uint8_t i = 5; i != 255; i--) {
				display(i);
				_delay_ms(1000);
			}
			estado = 2; // Cambiar a estado carrera
			break;
			
			case 2: // Carrera activa
			actualizarContadores();
			break;
			
			default: // Sistema inactivo
			display(0);
			PORTB &= 0xF0;
			PORTC &= 0xF0;
			break;
		}
	}
}

void actualizarContadores(void) {
	// Actualizar LEDs de los jugadores
	PORTB = (PORTB & 0xF0) | (jugador1 & 0x0F);
	PORTC = (PORTC & 0xF0) | (jugador2 & 0x0F);
}

ISR(PCINT0_vect) { // Interrupción botón inicio (PB4)
	if(!(PINB & (1 << PB4)) && (estado == 0)) {
		estado = 1;
	}
}

ISR(PCINT1_vect) { // Interrupción botones jugadores
	if(estado == 2) {
		if(!(PINC & (1 << PC4))) { // Jugador 1 (PC4)
			jugador1 = (jugador1 < 15) ? jugador1 + 1 : 0;
			_delay_ms(200);
		}
		if(!(PINC & (1 << PC5))) { // Jugador 2 (PC5)
			jugador2 = (jugador2 < 15) ? jugador2 + 1 : 0;
			_delay_ms(200);
		}
		actualizarContadores();
	}
}

void configurarInterrupciones(void) {
	PCICR |= (1 << PCIE0) | (1 << PCIE1);
	PCMSK0 |= (1 << PCINT4);     // PB4
	PCMSK1 |= (1 << PCINT12) | (1 << PCINT13); // PC4 y PC5
}