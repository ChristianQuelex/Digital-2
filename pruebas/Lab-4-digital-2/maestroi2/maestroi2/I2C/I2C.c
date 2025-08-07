/*
 * I2C.c
 *
 * Created: 1/08/2025 11:14:34
 *  Author: Chris Q
 */ 


#include "I2C.h"

//******************************************************************************************************************
//FUNCION PARA INICIALIZAR I2C Maestro
//******************************************************************************************************************
void I2C_Master_Init(unsigned long SCL_CLOCK, uint8_t Prescaler){
	DDRC &= ~((1<<DDC4)|(1<<DDC5));	//Se configuran los pines de I2C como entradas

	switch(Prescaler){
		case 1:
		TWSR &= ~((1<<TWPS1)|(1<<TWPS0));
		break;
		
		case 4:
			TWSR &= ~(1<<TWPS1);
			TWSR |= (1<<TWPS0);
		break;
		
		case 16:
			TWSR &= ~(1<<TWPS0);
			TWSR |= (1<<TWPS1);
		break;
		
		case 64:
			TWSR |= ((1<<TWPS1)|(1<<TWPS0));
		break;
		
		default:
			TWSR &= ~((1<<TWPS1)|(1<<TWPS0));
			Prescaler = 1;
		break;
		
	}
	
	TWBR = ((F_CPU/SCL_CLOCK)-16)/(2*Prescaler);		//MUST ...
	TWCR |= (1<<TWEN);

}

//******************************************************************************************************************
//FUNCION PARA EL INICIO DE LA COMUNICACION I2C
//******************************************************************************************************************

void I2C_Master_Start(void){
	uint8_t estado;
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);		//Se configura la condicion de Start
	while(!(TWCR & (1<<TWINT)));		//Espera a que termine la bandera TWINT
}

//******************************************************************************************************************
//FUNCION PARA LA PARADA DE LA COMUNICACION I2C
//******************************************************************************************************************

void I2C_Master_Stop(void){
	TWCR = (1<<TWEN)|(1<<TWINT)|(1<<TWSTO);		//Inicia la secuancia de parada del STOP
}

//*****************************************************************************************************************
//Funcion de transmision de datos del maestro al esclavo
//---> Esta funcion devolvera un 0 si el esclavo a recibido el dato
//*****************************************************************************************************************

uint8_t I2C_Master_Write(uint8_t dato){
	uint8_t estado;

	TWDR = dato; // Cargar el dato
	TWCR=(1 << TWEN)|(1 << TWINT); // Inicia el envio
	
	// Verificar si se transmitio una SLA + W con ACK, SLA + R con ACK, o un Dato con ACK
	if(estado == 0x18 || estado == 0x28 || estado == 0x40){
		return 1;
		}else{
		return estado;

	}
}

//*****************************************************************************************************************
//Funcion de recepcion de datos enviados por el esclavo al maestro
//----> Esta funcion es para leer los datos que estan en el esclavo
//*****************************************************************************************************************

uint8_t I2C_Master_Read(uint8_t *buffer, uint8_t ack){
	uint8_t estado;
	if(ack){
		TWCR |= (1 << TWEA); // Lectura con ACK
	}else{
		TWCR &= ~(1 << TWEA); // Lectura sin ACK
	}

	TWCR |= (1 << TWINT); // Iniciamos la lectura
	while(!(TWCR &(1 << TWINT)));		//Espera al flag TWINT 
	estado = TWSR & 0XF8;	 // Verificar estado
	// Verificar dato leido con ACK o sin ACK
	if(estado == 0x58 || estado == 0x50){
		*buffer = TWDR;
		return 1;
	}else{
		return estado;
	}
}

//******************************************************************************************************************
//FUNCION PARA INICIALIAR EL ESCLAVO
//******************************************************************************************************************

void I2C_Slave_Init(uint8_t address){
	DDRC &= ~((1 << DDC4)|(1 << DDC5));		// Pines de i2c como entradas --> Se debe implementar un Pull-Up con el nodo de 5V
											//Generlamente se emplea una resistencia de 4.7Kohms
									

	// Se habilita la interfaz, ACK automatico, se habilita la ISR
	TWCR=(1 << TWEA)|(1 << TWEN)|(1 << TWIE);
}