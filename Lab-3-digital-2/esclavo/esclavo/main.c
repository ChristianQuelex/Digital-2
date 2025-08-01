/*
 * esclavo.c
 *
 * Created: 31/07/2025 16:15:14
 * Author : Chris Q
 */ 



#include <avr/io.h>
#include <util/delay.h>

void ADC_init() {
	ADMUX |= (1 << REFS0); // AVcc como referencia
	ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // ADC enable, prescaler 128
}

uint16_t ADC_read(uint8_t channel) {
	ADMUX = (ADMUX & 0xF8) | (channel & 0x07); // Canal ADC0 o ADC1
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));
	return ADC;
}

void SPI_Slave_init() {
	DDRB |= (1 << DDB4); // MISO como salida
	SPCR |= (1 << SPE);  // Habilita SPI (modo esclavo)
}

int main() {
	ADC_init();
	SPI_Slave_init();
	uint16_t pot1, pot2;

	while (1) {
		pot1 = ADC_read(0); // Lee potenciómetro en PC0 (ADC0)
		pot2 = ADC_read(1); // Lee potenciómetro en PC1 (ADC1)

		// Envía los 4 bytes (2 por potenciómetro)
		SPDR = (pot1 >> 8) & 0xFF; // Byte alto de pot1
		while (!(SPSR & (1 << SPIF)));

		SPDR = pot1 & 0xFF; // Byte bajo de pot1
		while (!(SPSR & (1 << SPIF)));

		SPDR = (pot2 >> 8) & 0xFF; // Byte alto de pot2
		while (!(SPSR & (1 << SPIF)));

		SPDR = pot2 & 0xFF; // Byte bajo de pot2
		while (!(SPSR & (1 << SPIF)));

		_delay_ms(100);
	}
}