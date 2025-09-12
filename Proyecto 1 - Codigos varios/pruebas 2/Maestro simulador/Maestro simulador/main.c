#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "USART/USART.h"

// Variables simuladas
float peso_simulado = 0.0f;
float distancia_simulada = 0.0f;
float temperatura_simulada = 0.0f;

// Función para convertir float a string
void float_to_string(float num, char *buffer, int precision) {
	int integer_part = (int)num;
	float decimal_part = num - integer_part;
	
	// Convertir parte decimal
	for (int i = 0; i < precision; i++) {
		decimal_part *= 10.0f;
	}
	int decimal_int = (int)decimal_part;
	
	// Crear string
	if (precision > 0) {
		sprintf(buffer, "%d.%d", integer_part, decimal_int);
		} else {
		sprintf(buffer, "%d", integer_part);
	}
}

void setup(void) {
	initUART9600();
	DDRB |= (1 << DDB5);
	PORTB &= ~(1 << PORTB5);
	_delay_ms(1000);
	
	writeTextUART("Nano Simulator iniciado\n");
}

void simularDatosSensores(void) {
	peso_simulado = 500.0f + (rand() % 200 - 100);
	distancia_simulada = 50.0f + (rand() % 40 - 20);
	temperatura_simulada = 25.0f + (rand() % 40 - 20) * 0.1f;
}

void enviarDatosESP32(void) {
	char peso_str[10], dist_str[10], temp_str[10];
	
	// Convertir floats a strings
	float_to_string(peso_simulado, peso_str, 1);
	float_to_string(distancia_simulada, dist_str, 1);
	float_to_string(temperatura_simulada, temp_str, 1);
	
	// Crear JSON manualmente
	writeTextUART("{\"weight_g\": ");
		writeTextUART(peso_str);
		writeTextUART(", \"dist_cm\": ");
		writeTextUART(dist_str);
		writeTextUART(", \"temp_c\": ");
		writeTextUART(temp_str);
	writeTextUART("}\n");
	
	PORTB ^= (1 << PORTB5);
}

int main(void) {
	setup();
	
	while (1) {
		simularDatosSensores();
		enviarDatosESP32();
		_delay_ms(2000);
	}
}