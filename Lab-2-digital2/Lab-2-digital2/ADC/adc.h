/*
 * IncFile1.h
 *
 * Created: 18/07/2025 08:52:42
 *  Author: Chris Q
 */ 


#ifndef ADC_H_
#define ADC_H_

#define F_CPU 16000000
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


// Inicializaci?n
void initADC(void);

/* Seleccionar pin para ADC
	Seleccionar del 0 al 5
*/
void pinADC(uint8_t a);


// Mostrar ADC, del 0 al 7
uint8_t read_channelADC(uint8_t channel);


//iniciar conversi?n
void convertADC(void);

// mapeo de voltaje
float mapingADC(uint8_t channel);


#endif /* ADC_H_ */