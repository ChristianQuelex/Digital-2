/*
 * Lab-2-digital2.c
 *
 * Created: 18/07/2025 08:41:34
 * Author : Chris Q
 */ 

#define F_CPU 16000000
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include "other/LCD.h"
#include "ADC/adc.h"
#include "UART/uart.h"

// Inicialización de variables
uint8_t count = 0;
uint8_t counter = 0;
float voltage1 = 0;
float voltage2 = 0;
char buffer[10];  // Para almacenar la cadena del voltaje
volatile char bufferRX;

int main(void)
{
	cli();
	DDRC = 0;
	initLCD8bits();
	_delay_ms(20);
	
	initADC();
	pinADC(0); pinADC(1);   //PC0 Y PC1 como entradas ADC
	
	initUART9600();
	writeTextUART("Inicializacion completa\nControl del contador\n");
	writeTextUART("Formato: P1:XX.XXV P2:XX.XXV CNT:XXX\n");
	sei();
	
	LCD_Set_Cursor(1,2);    //row 1, column 2
	LCD_Write_String("P1:   P2:   P3:");    //Títulos en LCD

	while (1)
	{
		convertADC();       //start ADC conversion
		
		// Mostrar en LCD
		LCD_Set_Cursor(2,1);
		dtostrf(voltage1,4,2,buffer);   //Mostrar el primer valor
		LCD_Write_String(buffer);
		LCD_Write_String("V ");
		
		dtostrf(voltage2,4,2,buffer);   //Mostrar el segundo valor
		LCD_Write_String(buffer);
		LCD_Write_String("V  ");
		
		utoa(counter,buffer,10);    //Mostrar el contador
		LCD_Write_String("   ");
		LCD_Set_Cursor(2,14);
		LCD_Write_String(buffer);
		
		// Enviar por UART
		writeTextUART("P1:");
		dtostrf(voltage1,4,2,buffer);
		writeTextUART(buffer);
		writeTextUART("V P2:");
		dtostrf(voltage2,4,2,buffer);
		writeTextUART(buffer);
		writeTextUART("V CNT:");
		utoa(counter,buffer,10);
		writeTextUART(buffer);
		writeTextUART("\n");
		
		_delay_ms(500); // Retardo para no saturar la terminal
	}
}

ISR (ADC_vect){
	ADCSRA |= (1 << ADIF);  //turn off flag
	if (count == 0){
		count = 1;
		voltage1 = mapingADC(0);
		}else if(count == 1){
		count = 0;
		voltage2 = mapingADC(1);
	}
}

//ISR, recieve
ISR(USART_RX_vect){
	bufferRX = UDR0;
	while(!(UCSR0A & (1 << UDRE0)));   //if buffer is empty, if it is not, it waits
	serialShow(bufferRX);
	
	if(bufferRX == '+') counter++;
	else if(bufferRX == '-') counter--;
	else writeTextUART("Opcion invalida\n");
}