#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <util/delay.h>
#include "SPI/SPI.h"
#include "ADC/ADC.h"

// Prototipos de funciones locales
void setup(void);
void refreshLEDs(uint8_t value);
void testHardware(void);

int main(void) {
	setup();
	
	// Configurar SPI
	SPI_init(SPI_SLAVE, SPI_MSB_FIRST, SPI_CLOCK_IDLE_LOW, SPI_SAMPLE_ON_LEADING);
	SPI_enable_interrupt();
	sei();

	while(1) {
		// Todo se maneja en la interrupción SPI
	}
}

void setup(void) {
	// Configuración de LEDs
	DDRB |= (1 << DDB0) | (1 << DDB1);
	DDRD |= 0xFC;
	
	// Configuración de SS (PB2)
	DDRB &= ~(1 << DDB2);
	PORTB |= (1 << PORTB2);
	
	// Inicialización del ADC
	ADC_init();
	
	testHardware();
}

void refreshLEDs(uint8_t value) {
	PORTB = (PORTB & 0xFC) | ((value >> 6) & 0x03);
	PORTD = (value << 2) & 0xFC;
}

void testHardware(void) {
	// Test de LEDs
	for(uint8_t i=0; i<3; i++) {
		PORTB |= 0x03; PORTD |= 0xFC;
		_delay_ms(500);
		PORTB &= ~0x03; PORTD &= ~0xFC;
		_delay_ms(500);
	}
	
	// Test de ADC
	uint16_t val;
	uint32_t start = 0;
	while(start < 3000) {
		val = ADC_read(2);
		refreshLEDs(val >> 2);
		_delay_ms(100);
		start += 100;
	}
	refreshLEDs(0);
}

ISR(SPI_STC_vect) {
	static uint8_t command = 0;
	uint8_t received = SPI_receive();
	
	if(received == 'p') {
		command = 1;
		SPI_transmit(ADC_read(2) >> 2);
	}
	else if(command == 1) {
		SPI_transmit(ADC_read(3) >> 2);
		command = 0;
	}
	else {
		refreshLEDs(received);
		SPI_transmit(received);
	}
}