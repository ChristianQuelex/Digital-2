/*
 * I2C.h
 * Created: 27/02/2025 19:17:58
 * Author: Chris Q
 */

#ifndef I2C_H_
#define I2C_H_

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>   // Puertos del microcontrolador
#include <stdint.h>   // Tipos enteros de anchura fija

//******************************************************************************************************************
// FUNCIONES PARA CONFIGURAR EL MAESTRO
//******************************************************************************************************************

// Inicializar I2C Maestro: SCL_CLOCK en Hz (p.ej. 100000 para 100 kHz), Prescaler = 1,4,16,64
void I2C_Master_Init(unsigned long SCL_CLOCK, uint8_t Prescaler);

// Inicio de comunicación I2C (START)
void I2C_Master_Start(void);

// Parada de la comunicación I2C (STOP)
void I2C_Master_Stop(void);

// Transmisión de datos del maestro al esclavo
// ---> Devuelve 0 si el esclavo ACKea el dato; !=0 si NACK
uint8_t I2C_Master_Write(uint8_t dato);

// Recepción de datos enviados por el esclavo al maestro
// ---> Lee un byte del bus y lo coloca en *buffer
// ---> ack = 1 para responder con ACK (seguir leyendo), ack = 0 para NACK (último byte)
uint8_t I2C_Master_Read(uint8_t *buffer, uint8_t ack);

//******************************************************************************************************************
// FUNCION PARA INICIALIZAR I2C ESCLAVO
//******************************************************************************************************************
void I2C_Slave_Init(uint8_t address);

//******************************************************************************************************************
// Variables útiles para modo esclavo (opcionales)
//******************************************************************************************************************
// - I2C_SlaveData: dato que el maestro leerá del esclavo (SLA+R)
// - I2C_LastReceived: último dato que el esclavo recibió del maestro (SLA+W)
extern volatile uint8_t I2C_SlaveData;
extern volatile uint8_t I2C_LastReceived;

#endif /* I2C_H_ */
