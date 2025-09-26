/*
 * Parte de ATMega 328p.c
 *
 * Created: 12/09/2025 11:37:23
 * Author : Chris Q
 */ 


#define F_CPU 16000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <util/delay.h>
#include "UART/uart.h"

uint8_t start = 0;
uint8_t counter1 = 0;
uint8_t counter2 = 0;
uint8_t shift1 = 0;
uint8_t shift2 = 0;
uint8_t block = 0;

void initPCINT0(void);
void initPCINT1(void);

int main(void)
{
	cli();
	
	PORTB |= (1 <<  PINB0) | (1 << PINB1) | (1 <<  PINB2) | (1 <<  PINB3);		//pins of PORTB as input w pull up
	DDRB = 0x00;																//inputs and ouputs
	//PORTB |= (1 << PORTB5);													// Turn on integrated LED
	
	PORTC |= (1 << PINC4) | (1 << PINC5);										//pins of PORTB as input w pull up
	DDRC = 0x0F;																//inputs and outputs
	
	initPCINT0();
	initPCINT1();
	initUART9600();
	sei();
	
    while (1) 
    {

    }
}


//************************************************************
//                 Functions
//************************************************************

void initPCINT0(void){
	PCICR |= (1 << PCIE0);		//pin change
	PCMSK0 |= (1 << PCINT0) | (1 << PCINT1) | (1 << PCINT2) | (1 << PCINT3);	//mask
}

void initPCINT1(void){
	PCICR |= (1 << PCIE1);	
	PCMSK1 |= (1 << PCINT12) | (1 << PCINT13);
}



//************************************************************
//                 Interruptions
//************************************************************

// Directions
ISR (PCINT0_vect){
	if (!(PINB & (1 << PINB0)))	writeTextUART("0");		// Arriba
	if (!(PINB & (1 << PINB1)))	writeTextUART("1");		// Abajo
	if (!(PINB & (1 << PINB2)))	writeTextUART("2");		// Izquierda
	if (!(PINB & (1 << PINB3)))	writeTextUART("3");		// Derecha
}

// Actions
ISR(PCINT1_vect){
	if (!(PINC & (1 << PINC4))) writeTextUART("4");		// Acci?n 1
	if (!(PINC & (1 << PINC5))) writeTextUART("5");		// Acci?n 2
}