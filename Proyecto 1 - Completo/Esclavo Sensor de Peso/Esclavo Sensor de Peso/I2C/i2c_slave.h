/*
 * i2c_slave.h
 *
 * Created: 19/08/2025 13:19:16
 *  Author: Chris Q
 */ 


#ifndef I2C_SLAVE_H
#define I2C_SLAVE_H

#include <avr/io.h>
#include <avr/interrupt.h>

// Prototipos de funciones
void I2C_Slave_Init(uint8_t addr);

// Variables globales externas
extern volatile uint16_t tx_snap;
extern volatile uint8_t tx_idx;

#endif