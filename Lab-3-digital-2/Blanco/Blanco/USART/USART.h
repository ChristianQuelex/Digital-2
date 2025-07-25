/*
 * USART.h
 *
 * Created: 25/07/2025 05:12:55
 *  Author: Chris Q
 */ 




#ifndef USART_H_
#define USART_H_

#include <avr/io.h>
#include <stdint.h>

void initUART9600(void);

void writeUART(char caractrer);

void writeTextUART(char* texto);

unsigned char read_UART(void);


#endif /* USART_H_ */
