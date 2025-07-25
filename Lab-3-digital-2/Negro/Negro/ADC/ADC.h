/*
 * ADC.h
 *
 * Created: 25/07/2025 05:52:35
 *  Author: Chris Q
 */ 




#ifndef ADC_H_
#define ADC_H_

#include <avr/io.h>

void ADC_init(void);					//Prototipo de funci?n para configruar el ADC

uint16_t ADC_read(uint8_t canal);		//Prototipo de funci?n para leer los canales del ADC


#endif /* ADC_H_ */