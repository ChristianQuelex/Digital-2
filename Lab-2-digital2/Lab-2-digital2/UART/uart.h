/*
 * IncFile1.h
 *
 * Created: 18/07/2025 08:52:59
 *  Author: Chris Q
 */ 





#ifndef UART_H_
#define UART_H_

#define F_CPU 16000000
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


// Inicializaci?n del m?dulo UART
void initUART9600(void);


// Escribir una cadena
void writeTextUART(char* text);


// Muestra en el monitor serie
void serialShow(volatile char buffer);



#endif /* UART_H_ */