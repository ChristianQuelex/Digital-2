/*
 * USART.c
 *
 * Created: 27/02/2025 19:48:33
 *  Author: Chris Q
 */ 

#include "USART.h"

//Inicializar la funci?n del UART
void initUART9600(void){
	//Paso1: Configurar pines TX y RX
	DDRD &= ~(1<<DDD0);		//Se configura el RX (PD0) como entrada
	DDRD |= (1<<DDD1);		//Se configura el TX (PD1) como salida
	
	//Paso2: Configurar registro A ---> Modo FAST U2X0 = 1
	UCSR0A = 0;
	UCSR0A |= (1<<U2X0);
	
	//Paso3: Configurar registro B --> Habilitar ISR RX, se habilita RX y TX
	//->>Se configura para tener interrupciones
	UCSR0B = 0;		//Se pone en 0 para mayor facilidad de configurar posteriomente
	UCSR0B |= (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0);
	
	//Paso4: Configurar C > Frame (Se define el frame): 8 bits datos, no paridad, 1 bit de stop
	UCSR0C = 0;    //--> Se configura si se desea modo: asincrono o sincrono
	UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00);	//Se configura el tama?o del caracter --> 8 bits
	
	//Paso5: Baudrate = 115200
	UBRR0 = 16;
}

void writeUART(char caractrer){
	while(!(UCSR0A &(1<<UDRE0)));		//La funci?n se queda en espera hasta que UDR este en 1
	UDR0 = caractrer;
}

void writeTextUART(char* texto){			// "*" es un puntero del char
	uint8_t i;
	for(i = 0; texto[i]!= '\0'; i++){
		while(!(UCSR0A &(1<<UDRE0)));		//La funci?n se queda en espera hasta que UDR este en 1
		UDR0 = texto[i];
	}
}

// Funci?n para leer un car?cter por UART
unsigned char read_UART(void) {
	if (UCSR0A & (1 << RXC0)) {  // Verificar si el dato est? disponible
		return UDR0;  // Retornar el dato recibido
		} else {
		return 0;  // Si no hay datos, retornar 0
	}
}

// Funci?n para recibir una cadena de caracteres por UART
void readStringUART(char* str) {
	unsigned char i = 0;
	unsigned char received_char;

	while (1) {
		received_char = read_UART();  // Leer un car?cter

		if (received_char == '\n' || received_char == '\r') {  // Fin de la cadena
			str[i] = '\0';  // Terminar la cadena
			break;
		}

		if (received_char != 0) {  // Si el car?cter no es 0
			str[i++] = received_char;  // Almacenar en la cadena
		}
	}
}