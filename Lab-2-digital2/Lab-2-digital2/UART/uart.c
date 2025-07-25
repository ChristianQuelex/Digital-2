/*
 * CFile1.c
 *
 * Created: 18/07/2025 08:52:52
 *  Author: Chris Q
 */ 




#include "uart.h"

void serialShow(volatile char buffer){
	UDR0 = buffer;
	writeTextUART(" <-- Enviado \n");
}

void writeTextUART(char* text){
	for (uint8_t i = 0; text[i] != '\0'; i++)
	{
		while(!(UCSR0A & (1 << UDRE0)));
		UDR0 = text[i];
	}
}


void initUART9600(void){
	//settigns for RX and TX
	DDRD &= ~(1 << DDD0);		//Rx as input
	DDRD |= (1 << DDD1);		//TX as output
	
	//Fast mode, U2X0
	UCSR0A = 0;
	UCSR0A |= (1 << U2X0);
	
	//Settigns for register B
	UCSR0B = 0;
	UCSR0B |= (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0); //ISR, enable for RX and TX
	
	// settigns for register C
	UCSR0C = 0;
	UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);		// character size: 8 bits, no parity, 1 stop bit
	
	//Baudrate
	UBRR0 = 207;		// 9600
}