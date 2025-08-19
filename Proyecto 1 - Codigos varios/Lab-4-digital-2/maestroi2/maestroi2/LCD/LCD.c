/*
 * lcd.c
 *
 * Created: 1/08/2025 11:09:07
 *  Author: Chris Q
 */ 


#include "LCD.h"


#include <avr/io.h>
#include <util/delay.h>

// Funcion para inicializar la LCD en modo 8 bits
void initLCD8bits(void){
	// Configurar los pines como salida
	DDRB |= (1 << DDB0) | (1 << DDB1) | (1 << DDB2) | (1 << DDB3); // D6, D7, RS, E
	DDRD |= (1 << DDD2) | (1 << DDD3) | (1 << DDD4) | (1 << DDD5) | (1 << DDD6) | (1 << DDD7); // D0-D5

	// Asegurar que los puertos inicien en 0
	PORTB = 0;
	PORTD = 0;

	// Secuencia de inicializacion de la LCD
	_delay_ms(20);
	LCD_CMD(0x30);
	_delay_ms(5);
	LCD_CMD(0x30);
	_delay_ms(1);
	LCD_CMD(0x30);
	_delay_ms(1);
	LCD_CMD(0x38); // Modo 8 bits, 2 l?neas, 5x8 puntos

	LCD_CMD(0x0C); // Encender display sin cursor
	LCD_CMD(0x06); // Modo de entrada: incrementar direcci?n, sin desplazamiento
	LCD_CMD(0x01); // Limpiar pantalla
	_delay_ms(2);
}

// Funcion para enviar un comando a la LCD
void LCD_CMD(char a) {
	PORTB &= ~(1 << PORTB2); // RS = 0 (modo comando)
	LCD_Port(a);
	PORTB |= (1 << PORTB3);  // E = 1
	_delay_ms(1);
	PORTB &= ~(1 << PORTB3); // E = 0
}

// Funcion para escribir en los pines de datos de la LCD
void LCD_Port(char a) {
	// Bits menos significativos en PORTD (D0-D5)
	PORTD = (PORTD & 0x03) | ((a & 0x3F) << 2);
	// Bits m?s significativos en PORTB (D6-D7)
	PORTB = (PORTB & 0xFC) | ((a >> 6) & 0x03);
}

// Funcion para escribir un car?cter en la LCD
void LCD_Write_Char(char c){
	PORTB |= (1 << PORTB2); // RS = 1 (modo datos)
	LCD_Port(c);
	PORTB |= (1 << PORTB3); // E = 1
	_delay_ms(1);
	PORTB &= ~(1 << PORTB3); // E = 0
}

// Funcion para escribir una cadena de caracteres en la LCD
void LCD_Write_String(char *a) {
	while (*a) {
		LCD_Write_Char(*a);
		a++;
	}
}

// Funcion para posicionar el cursor en la LCD
void LCD_Set_Cursor(char c, char f) {
	char temp = (f == 1) ? (0x80 + c - 1) : (0xC0 + c - 1);
	LCD_CMD(temp);
}

// Funcion para limpiar la LCD
void LCD_Clear(void) {
	LCD_CMD(0x01);
	_delay_ms(2);
}

// Funcion para desplazar la pantalla a la derecha
void LCD_Shift_Right(void) {
	LCD_CMD(0x1C);
}

// Funcion para desplazar la pantalla a la izquierda
void LCD_Shift_Left(void) {
	LCD_CMD(0x18);
}