#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include "USART./USART.h"
#include "SPI/SPI.h"

void setup(void) {
	// Configurar LEDs
	DDRB |= (1 << DDB0) | (1 << DDB1);
	DDRD |= (1 << DDD2) | (1 << DDD3) | (1 << DDD4) | (1 << DDD5) | (1 << DDD6) | (1 << DDD7);
	
	// Configurar SS (PC0)
	DDRC |= (1 << DDC0);
	PORTC |= (1 << PORTC0);
}

void refreshLEDs(uint8_t value) {
	PORTB = (PORTB & 0xFC) | ((value >> 6) & 0x03);
	PORTD = (PORTD & 0x03) | ((value << 2) & 0xFC);
}

uint8_t readPots(void) {
	PORTC &= ~(1 << PORTC0); // Activar esclavo (SS bajo)
	_delay_us(10);
	
	SPI_transmit('p'); // Enviar comando
	uint8_t pot1 = SPI_receive(); // Leer Pot1
	uint8_t pot2 = SPI_receive(); // Leer Pot2
	
	PORTC |= (1 << PORTC0); // Desactivar esclavo (SS alto)
	
	// Mostrar valores por UART
	char buffer[32];
	sprintf(buffer, "P1:%d P2:%d\n", pot1, pot2);
	writeTextUART(buffer);
	
	return pot1;
}

int main(void) {
	setup();
	
	// Inicializar SPI como MASTER
	SPI_init(SPI_MASTER, SPI_MSB_FIRST, SPI_CLOCK_IDLE_LOW, SPI_SAMPLE_ON_LEADING);
	
	// Inicializar UART
	initUART9600();
	
	writeTextUART("Sistema Maestro listo\n");
	writeTextUART("Envie:\n- 'p' para potenciometros\n- 0-255 para LEDs\n");

	while(1) {
		if(UCSR0A & (1 << RXC0)) {
			uint8_t data = UDR0;
			
			if(data == 'p') {
				readPots();
				} else {
				refreshLEDs(data);
				
				// Enviar al esclavo
				PORTC &= ~(1 << PORTC0);
				_delay_us(10);
				SPI_transmit(data);
				PORTC |= (1 << PORTC0);
				
				// Confirmación
				writeTextUART("LED:");
				char num[4];
				itoa(data, num, 10);
				writeTextUART(num);
				writeTextUART("\n");
			}
		}
	}
}