/*
 * UART.h
 *
 * Created: 31/07/2025 23:00:46
 *  Author: Chris Q
 */ 


#ifndef UART_H
#define UART_H

#include <avr/io.h>

void uart_init(uint16_t baud_rate);
void uart_print(char *str);
void uart_print_number(uint16_t num);
uint8_t uart_receive_number();

#endif