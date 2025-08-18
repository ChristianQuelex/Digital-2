#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ========================
// HX711
// ========================
#define HX711_DOUT   PD2
#define HX711_PD_SCK PD3

long tare_offset = 0;
float scale_factor = 1.0;

// EEPROM
uint32_t EEMEM ee_tare_offset;
float    EEMEM ee_scale_factor;

// ========================
// HX711 funciones
// ========================
long HX711_read(void) {
	long value = 0;
	while (PIND & (1 << HX711_DOUT)); // esperar a que DOUT se ponga en 0

	for (uint8_t i = 0; i < 24; i++) {
		PORTD |= (1 << HX711_PD_SCK);
		_delay_us(1);
		value = (value << 1) | ((PIND & (1 << HX711_DOUT)) ? 1 : 0);
		PORTD &= ~(1 << HX711_PD_SCK);
		_delay_us(1);
	}
	// pulso extra para ganancia
	PORTD |= (1 << HX711_PD_SCK);
	_delay_us(1);
	PORTD &= ~(1 << HX711_PD_SCK);

	if (value & 0x800000) value |= 0xFF000000; // signo extendido
	return value;
}

long HX711_read_average(uint8_t times) {
	long sum = 0;
	for (uint8_t i = 0; i < times; i++) {
		sum += HX711_read();
	}
	return sum / times;
}

float HX711_get_units(uint8_t times) {
	long reading = HX711_read_average(times);
	return (float)(reading - tare_offset) / scale_factor;
}

// ========================
// EEPROM funciones
// ========================
void save_calibration(void) {
	eeprom_update_dword(&ee_tare_offset, tare_offset);
	eeprom_update_float(&ee_scale_factor, scale_factor);
}

void load_calibration(void) {
	tare_offset = eeprom_read_dword(&ee_tare_offset);
	scale_factor = eeprom_read_float(&ee_scale_factor);
	if (scale_factor <= 0.0f) scale_factor = 1.0f;
}

// ========================
// I2C (TWI) Slave
// ========================
#define SLA_ADDR 0x08   // dirección del esclavo

volatile char tx_buffer[16]; // para mandar peso en texto
volatile uint8_t tx_index = 0;
volatile uint8_t tx_length = 0;

void I2C_init(uint8_t address) {
	TWAR = (address << 1);  // dirección esclavo
	TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT) | (1 << TWIE);
}

ISR(TWI_vect) {
	switch (TWSR & 0xF8) {
		case 0x60: // Own SLA+W recibido
		case 0x68:
		TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT) | (1 << TWIE);
		break;

		case 0xA8: // SLA+R recibido, maestro quiere leer
		case 0xB0:
		tx_index = 0;
		tx_length = strlen((char*)tx_buffer);
		if (tx_length == 0) {
			strcpy((char*)tx_buffer, "0");
			tx_length = 1;
		}
		TWDR = tx_buffer[tx_index++];
		TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT) | (1 << TWIE);
		break;

		case 0xB8: // Maestro sigue leyendo
		if (tx_index < tx_length) {
			TWDR = tx_buffer[tx_index++];
			} else {
			TWDR = 0;
		}
		TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT) | (1 << TWIE);
		break;

		case 0xC0: // Maestro dejó de leer
		case 0xC8:
		TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT) | (1 << TWIE);
		break;

		default:
		TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT) | (1 << TWIE);
		break;
	}
}

// ========================
// Main
// ========================
int main(void) {
	// Configurar HX711 pines
	DDRD &= ~(1 << HX711_DOUT);
	DDRD |= (1 << HX711_PD_SCK);

	load_calibration(); // cargar tare + scale

	sei();              // habilitar interrupciones
	I2C_init(SLA_ADDR);

	char buffer[16];

	while (1) {
		float peso = HX711_get_units(10);
		dtostrf(peso, 0, 2, buffer);   // convertir a string con 2 decimales
		strcpy((char*)tx_buffer, buffer);
		_delay_ms(500);                // refrescar cada 0.5s
	}
}
