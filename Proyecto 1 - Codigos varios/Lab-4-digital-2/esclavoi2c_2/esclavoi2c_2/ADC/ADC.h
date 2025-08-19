/*
 * ADC.h
 *
 * Created: 7/08/2025 06:32:00
 *  Author: Chris Q
 */ 



#ifndef ADC_H_
#define ADC_H_

#include <avr/io.h>
#include <stdint.h>

void ADC_init(void);					//Prototipo de funci?n para configruar el ADC

uint16_t ADC_read(uint8_t canal);		//Prototipo de funci?n para leer los canales del ADC

#endif /* ADC_H_ */