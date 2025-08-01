/*
 * USART.h
 *
 * Created: 25/07/2025 05:12:55
 *  Author: Chris Q
 */ 


#ifndef USART_H
#define USART_H

#include <avr/io.h>
#include <stdint.h>

void initUART9600(void);
void writeUART(char character);
void writeTextUART(char* text);
unsigned char read_UART(void);

#endif