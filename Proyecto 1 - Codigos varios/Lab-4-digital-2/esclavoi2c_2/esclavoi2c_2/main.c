/*
 * esclavoi2c_2.c
 *
 * Created: 7/08/2025 06:30:01
 * Author : Chris Q
 */ 


#define F_CPU 16000000		// Definir CPU con el que trabajara el microcontrolador
#include <avr/io.h>			// Se incluye la libreria para poder utilizar los puertos del microcontrolador
#include <util/delay.h>		// Se incluye la libreria que implementa los delay
#include <avr/interrupt.h>	//Se incluye las librerias de las interrupciones


//Se incluyen las Librerias Propias
#include "I2C/I2C.h"
#include "ADC/ADC.h"
//Se define la dirrecion de la comunicacion (Maesro a Esclavo)
#define SlaveAddress 0x30	

//Se definen los prototipos de funcion

void refreshPORT(uint8_t valor);	//Prototipo de funcion para las salidas


//VARIABLES GLOBALES

uint8_t buffer = 0;		//Variable que recibe el "comando" del maestro
volatile uint8_t valorADC = 0;

//LOOP


int main(void)
{
	DDRB |=(1 << DDB5);
	PORTB &= ~(1 << PORTB5);
	ADC_init();

	I2C_Slave_Init(SlaveAddress);		//Se inicia el esclavo con la direccion del maestro designada

	sei();		// Habilitar Interrupciones generales

 
    while (1) 
    {

		// Iniciando secuencia de adc
		valorADC = ADC_read(0);
		if(buffer == 'R'){
			PINB |= (1 << PINB5);		//El led parpade si se establece conexion
			buffer = 0;
		}
	
	}
}


//INTERRUPCIONES


ISR(ADC_vect){
	ADCSRA |= (1 << ADIF);	//Limpiar la bandera al encender el ADC
}


ISR(TWI_vect){
	uint8_t estado;
	estado = TWSR & 0xFC;
	switch(estado){
		case 0x60:
		case 0x70:
		TWCR |=(1 << TWINT);
		break;
		case 0x80:
		case 0x90:
		buffer = TWDR;		//Se carga el "comando" enviado por el maestro
		TWCR |= (1 << TWINT); // Se limpia la bandera
		break;
		case 0xA8:
		case 0xB8:
		TWDR = valorADC; // Cargar el dato de la ADC
		TWCR = (1 << TWEN)|(1 << TWIE)|(1 << TWINT)|(1 << TWEA); // Inicia el envio
		break;
		default: // Se libera el bus de cualquier error
		TWCR |=(1 << TWINT)|(1 << TWSTO);
		break;
	}
}
		