/*
 * USART.h
 *
 * Created: 27/02/2025 19:48:19
 *  Author: Chris Q
 */ 


#ifndef USART_H_
#define USART_H_

#include <avr/io.h>			// Se incluye la libreria para poder utilizar los puertos del microcontrolador
#include <stdint.h>			// Declara conjuntos de tipos enteros que tienen anchuras especificadas

void initUART9600(void);			// Prototipo de funci?n - Inicializar Comunicaci?n Serial

void writeUART(char caractrer);		// Prototipo de funci?n - Escribir caracter en USART

void writeTextUART(char* texto);	// Prototipo de funci?n - Escribir cadena de caracteres en USART

unsigned char read_UART(void);		// Prototipo de funci?n - Leer desde USART

char UART_Receive(void);			// Prototipo de funci?n - Recibir un Byte

void readStringUART(char* str);	//Prototipo de funci?n - Recibir cadena de caracteres desde USART

#endif /* USART_H_ */