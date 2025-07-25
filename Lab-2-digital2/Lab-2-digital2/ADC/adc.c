/*
 * CFile1.c
 *
 * Created: 18/07/2025 08:52:28
 *  Author: Chris Q
 */ 




#include "adc.h"

// Inicializaci?n
void initADC(void){
	ADMUX = 0;
	//Vref = AVcc = 5Vs
	ADMUX |= (1 << REFS0);
	ADMUX &= ~(1 << REFS1);
		
	ADMUX |= (1 << ADLAR);	//left adjust
		
	ADCSRA = 0;
	ADCSRA |= (1 << ADEN);	//turn on ADC
	ADCSRA |= (1 << ADIE);	//interruption
		
	//prescaler 128 > 125kHz
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

// Seleccionar pin para ADC, habilita los necesarios
void pinADC(uint8_t a){
	switch (a)
	{
	case 0:
	DIDR0 |= (1 << ADC0D);
	break;
	
	case 1:
	DIDR0 |= (1 << ADC1D);
	break;
	
	case 2:
	DIDR0 |= (1 << ADC2D);
	break;
	
	case 3:
	DIDR0 |= (1 << ADC3D);
	break;
	
	case 4:
	DIDR0 |= (1 << ADC4D);
	break;
	
	case 5:
	DIDR0 |= (1 << ADC5D);
	break;
	}
}


// Mostrar ADC
uint8_t read_channelADC(uint8_t channel){
	switch (channel)
	{
	case 0:
	ADMUX = (ADMUX & 0xF0); // mantiene los primeros 4 bits de config y setea canal de conversi?n
	break;
	
	case 1:
	ADMUX = (ADMUX & 0xF0) | 0x01;
	break;
	
	case 2:
	ADMUX = (ADMUX & 0xF0) | 0x02;
	break;
	
	case 3:
	ADMUX = (ADMUX & 0xF0) | 0x03;
	break;
	
	case 4:
	ADMUX = (ADMUX & 0xF0) | 0x04;
	break;
	
	case 5:
	ADMUX = (ADMUX & 0xF0) | 0x05;
	break;
	
	case 6:
	ADMUX = (ADMUX & 0xF0) | 0x06;
	break;
	
	case 7:
	ADMUX = (ADMUX & 0xF0) | 0x07;
	break;
	
	case 8:
	ADMUX = (ADMUX & 0xF0) | 0x08;
	break;
	}
	
	return ADCH;
}


//iniciar conversi?n
void convertADC(void){
	ADCSRA |= (1 << ADSC);
	_delay_ms(2);
}



float mapingADC(uint8_t channel){
	uint8_t adcValue = 0;
	float voltage = 0;
	adcValue = read_channelADC(channel);  // Leer ADC en el canal 3

	// Convertir el valor ADC a voltaje
	return voltage = (adcValue * 5.0) / 256.0;
}
