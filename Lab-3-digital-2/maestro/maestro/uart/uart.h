/*
 * uart.h
 *
 * Created: 1/08/2025 07:09:50
 *  Author: Chris Q
 */ 


#ifndef UART_H
#define UART_H

#include <avr/io.h>

void uart_init(uint16_t baud_rate);
void uart_print(const char *str);
void uart_print_number(uint16_t num);

#endif