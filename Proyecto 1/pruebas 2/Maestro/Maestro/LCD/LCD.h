/*
 * LCD.h
 *
 * Created: 27/02/2025 19:47:30
 *  Author: Chris Q
 */ 


#ifndef LCD_H_
#define LCD_H_

#define F_CPU 16000000
#include <avr/io.h>			// Se incluye la libreria para poder utilizar los puertos del microcontrolador
#include <util/delay.h>		// Se incluye la libreria que implementa los delay

//Se puede definir un puerto como una letra para mayor facilidad al momento de programar
#define E (1 << PORTB3)			
#define RS (1 << PORTB2)

//Funcion para inicializar LCD en modo 4 bits
void initLCD8bits(void);
// Funcion para colocar en el puerto un valor
void LCD_Port(char a);
// Funcion para enviar un comando
void LCD_CMD(char a);
// Funcion para enviar un caracter
void LCD_Write_Char(char c);
// Funcion para enviar una cadena
void LCD_Write_String(char *a);
// Desplazamiento hacia la derecha
void LCD_Shift_Right(void);
// Desplazamiento hacia la izquierda
void LCD_Shift_Left(void);
// Establecer el cursor
void LCD_Set_Cursor(char c, char f);

void LCD_Clear(void);


#endif /* LCD_H_ */