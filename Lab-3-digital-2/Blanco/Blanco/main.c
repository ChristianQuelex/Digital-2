/*
 * Blanco.c
 *
 * Created: 25/07/2025 05:08:40
 * Author : Chris Q
 */ 


#define F_CPU 16000000		// Definir CPU con el que trabajara el microcontrolador
							//-	Debe estar al inicio de la progra
	
//Se definen variables clave						
#define  entrada0 48
#define  entrada1 49
#define  entrada2 50

#include <avr/io.h>			// Se incluye la libreria para poder utilizar los puertos del microcontrolador
#include <avr/interrupt.h>	//Se incluye las librerias de las interrupciones
#include <stdint.h>			// Declara conjuntos de tipos enteros que tienen anchuras especificadas
#include <util/delay.h>		// Se incluye la libreria que implementa los delay

#include <stdlib.h>

#include <string.h>

//Se llaman a las librerias propias
#include "SPI/SPI.h"
#include "USART/USART.h"

//Se declaran variables
uint8_t valorSPI = 0;

//Almacenar los valores convertido a texto
char POT1[16];
char POT2[16];


char buffer[10]; // Buffer para almacenar la conversi?n del n?mero a texto

//Declarar variables volatiles
volatile char bufferRX;				//Se pine volatile a las variables que pueden cambiar en cualquier momento
volatile uint8_t input = 0;			//Se declara la variable para selecionar los datos [Potenciometro o LED]
volatile uint8_t inputpot = 0;		//Se declara la variable para selecionar los datos [Potenciometro 1 o 2]
volatile uint8_t flag = 1;			//Se declara la bandera para el menu

uint8_t contador = 0;

//Prototipo de funciones
void refreshPORT(uint8_t valor);

void float_to_string(float num, char *buffer, int precision);

void setup(void);

//******************************************************************************************************************
//LOOP
//******************************************************************************************************************
int main(void)
{
	cli();			//Se  apagan las interupcciones Globales
	
	//Se llama a la funci?n de configuraci?n de los puertos de las LEDS y la Comunicaci?n SPI
	setup();
	
	//Se llama a la funci?n de la SPI
	spiInit(SPI_MASTER_OSC_DIV16,SPI_DATA_ORDER_MSB, SPI_CLOCK_IDLE_LOW, SPI_CLOCK_FIRST_EDGE);
	//spiInit(SPI MASTER OSC DIV16,SPI DATA ORDER MSB, SPI CLOCK IDLE LOW,SPI CLOCK FIRST_EDGE)

	initUART9600();		//Se llama a la funci?n de UART
	
	sei();			//Se  inician las interupcciones Globales
	
	//Mandar caracteres al UART
	writeUART('H');
	writeUART('o');
	writeUART('l');
	writeUART('a');
	writeUART(' ');
	
	//Mandar cadena de caracteres al UART
	writeTextUART("\n\nMuchas gracias por usar el programa");
	

    while (1) 
    {
		
		//Configuraci?n menu de Inicio
		if(flag == 1){
			input = 0;
			//Mostrar Menu
			writeTextUART("\n\n-->?C?al de las siguientes opciones quiere realizar?\n");
			writeTextUART("[1]Leer el potenciometro 1\n");
			writeTextUART("[2]Comprobar LEDS 2\n");
			writeTextUART("-->Selecione una opci?n: ");
			
			//El programa espera hasta que haya ingresado un valor
			while(!(input == entrada1 || input == entrada2));		//Se espera hasta que se cumpla la condici?n
			
			//Verifica que se haya cumplido cualquiera de las dos condiciones
			switch(input){
				
				case entrada1: //Se elige la primera Opci?n - [Potenciometros 1 o 2]
				
				spiWrite('c');      // Enviar solicitud de lectura SPI
				valorSPI = spiRead(); // Leer el valor SPI
				
				// Enviar el valor le?do por USART
				char buffer[10];
				itoa(valorSPI, buffer, 10); // Convierte valorSPI a texto
				writeTextUART("\nValor SPI recibido: ");
				writeTextUART(buffer);
				writeTextUART("\n");
				
				flag = 0;			//Set la bandera en 0
				_delay_ms(20); //Espera a que la operaci?n termine
				break;
				
				case entrada2: //Se elige la opci?n de la (ASCII)
				writeTextUART("\n\n-->Se inicia el contador de LEDS ");		//Ingresar el valor deL ASCII
				
				
				for (uint8_t contador = 0; contador < 255; contador++){
					PORTC &= ~(1 << PORTC0); // SLAVE SELECT
					spiWrite(contador);
					valorSPI = spiRead();
					refreshPORT(valorSPI);
					
					PORTC |= (1 << PORTC0); // SLAVE SELECT
					
					_delay_ms(250);
				}
				
				flag = 0;      //Set la bandera en 0
				break;
			}
		}
		flag = 1;
		
		contador = 0;
		
		refreshPORT(0);
		
		
		_delay_ms(100);
    }
}

//******************************************************************************************************************
//FUNIONES
//******************************************************************************************************************



void setup(void){
	//Se configuran los puertos como saldias
	DDRD |= (1 << DDD2)|(1 << DDD3)|(1 << DDD4)|(1 << DDD5)|(1 << DDD6)|(1 << DDD7);
	DDRB |=(1 << DDB0)|(1 << DDB1)|(1 << DDB2);
	DDRC |= (1 << DDC0);
	PORTC |= (1 << PORTC0);
	
	PORTD |= ~((1 << PORTD2)|(1 << PORTD3)|(1 << PORTD4)|(1 << PORTD5)|(1 << PORTD6)|(1 << PORTD7));
	PORTB &= ~((1 << PORTB0)|(1 << PORTB1));
}

void refreshPORT(uint8_t valor){
	if(valor & 0b10000000){
		PORTB |=(1 << PORTB1);
	}else{
		PORTB &= ~(1 << PORTB1);
	}
	if(valor & 0b01000000){
		PORTB |=(1 << PORTB0);
	}else{
		PORTB &= ~(1 << PORTB0);
	}
	if(valor & 0b00100000){
		PORTD |= (1 << PORTD7);
	}else{
		PORTD &= ~(1 << PORTD7);
	}
	if(valor & 0b00010000){
		PORTD |= (1 << PORTD6);
	}else{
		PORTD &= ~(1 << PORTD6);
	}
	if(valor & 0b00001000){
		PORTD |= (1 << PORTD5);
	}else{
		PORTD &= ~(1 << PORTD5);
	}
	if(valor & 0b00000100){
		PORTD |= (1 << PORTD4);
	}else{
		PORTD &= ~(1 << PORTD4);
	}
	if(valor & 0b00000010){
		PORTD |= (1 << PORTD3);
	}else{
		PORTD &= ~(1 << PORTD3);
	}
	if(valor & 0b00000001){
		PORTD |= (1 << PORTD2);
		}else{
		PORTD &= ~(1 << PORTD2);
	}
	
}

//******************************************************************************************************************
//INTERRUPCIONES
//******************************************************************************************************************

//Configuraci?n despues de haber leido el menu
ISR (USART_RX_vect){
	bufferRX = UDR0;
	
	input = bufferRX;
	
	if(flag == 1){
		if(input == entrada1){
			writeTextUART("\n\nLeyendo el valor del potenciometro... ");
		}
		
		else if(input == entrada2){
			writeTextUART("\n\nLeyendo el valor de ASCII... ");
		}
		
		else{
			writeTextUART("\n\nEntrada inv?lida. Por favor ingrese 1 o 2.");
			writeTextUART("\n\n-->Ingrece nuevamente la opci?n: ");
		}
	}
	
	if(flag == 0){
		//Configuraci?n salidas del ASCII (LEDS)
		flag = 1;
	}
}
