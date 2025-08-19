/*
 * I2C.h
 *
 * Created: 27/02/2025 19:17:58
 *  Author: Guillermo Schwartz
 */


#ifndef I2C_H_
#define I2C_H_

#ifndef F_CPU
#define F_CPU 16000000
#endif

#include <avr/io.h>			// Se incluye la libreria para poder utilizar los puertos del microcontrolador
#include <stdint.h>			// Declara conjuntos de tipos enteros que tienen anchuras especificadas


//******************************************************************************************************************
//FUNCIONES PARA CONFIGURAR EL MAESTRO
//******************************************************************************************************************

//Funci?n para inicializar I2C Maestro
void I2C_Master_Init(unsigned long SCL_CLOCK, uint8_t Prescaler);

//Funci?n para el inicio de la comunicaci?n I2C
void I2C_Master_Start(void);

//Funci?n para la parada de la comunicaci?n I2C
void I2C_Master_Stop(void);

//Funcion de transmision de datos del maestro al esclavo

//---> Esta funcion devolvera un 0 si el esclavo a recibido el dato
uint8_t I2C_Master_Write(uint8_t dato);

//Funcion de recepcion de datos enviados por el esclavo al maestro

//---> Esta funcion es para leer los datos que estan en el esclavo
uint8_t I2C_Master_Read(uint8_t *buffer, uint8_t ack);


//******************************************************************************************************************
//FUNCION PARA INICIALIZAR I2C ESCLAVO
//******************************************************************************************************************
void I2C_Slave_Init(uint8_t address);

#endif /* I2C_H_ */