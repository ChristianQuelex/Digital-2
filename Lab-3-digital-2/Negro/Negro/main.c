/*
 * Negro.c
 *
 * Created: 25/07/2025 05:30:34
 * Author : Chris Q
 */ 


#define F_CPU 16000000		// Definir CPU con el que trabajara el microcontrolador
							//-	Debe estar al inicio de la progra

#include <avr/io.h>			// Se incluye la libreria para poder utilizar los puertos del microcontrolador
#include <avr/interrupt.h>	//Se incluye las librerias de las interrupciones
#include <stdint.h>			// Declara conjuntos de tipos enteros que tienen anchuras especificadas
#include <util/delay.h>		// Se incluye la libreria que implementa los delay

//Se llaman a las librerias propias
#include "SPI/SPI.h"
#include "ADC/ADC.h"


//Se declaran variables
uint8_t valorSPI = 0;

//Prototipo de funciones
void refreshPORT(uint8_t valor);

void setup(void);


int main(void)
{
	cli();
	//Se llama a la funci?n de configuraci?n de los puertos
	setup();
	
	//Se llama a la funci?n del ADC
	
	//Se llama a la funci?n de la SPI
	
	spiInit(SPI_SLAVE_SS,SPI_DATA_ORDER_MSB, SPI_CLOCK_IDLE_LOW, SPI_CLOCK_FIRST_EDGE);
	//spiInit(SPI MASTER OSC DIV16,SPI DATA ORDER MSB, SPI CLOCK IDLE LOW,SPI CLOCK FIRST_EDGE)
	
	//Se habilita el m?dulo SPI
	SPCR |= (1<<SPIE);
	
	//Se activan las interrupcoiones goblaes
	sei();

    while (1) 
    {

    }
}

//
//FUNIONES
//

void setup(void){
	//Se configuran los puertos como saldias
	DDRD |= (1 << DDD2)|(1 << DDD3)|(1 << DDD4)|(1 << DDD5)|(1 << DDD6)|(1 << DDD7);
	DDRB |=(1 << DDB0)|(1 << DDB1)|(1 << DDB2);
	PORTB |= (1 << PORTB2);
	
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

ISR(SPI_STC_vect){
	uint8_t spiValor = SPDR;
	if(spiValor == 'c'){
		uint8_t ADC_Valor = ADC_read(3);
		spiWrite(ADC_Valor);
	}
}

ISR(ADC_vect){
	ADCSRA |= (1 << ADIF);	//Limpiar la bandera al encender el ADC
}