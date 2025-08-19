//******************************************************************************************************************
// UNIVERSIDAD DEL VALLE DE GUATEMALA
// IE2023: PROGRAMACIÓN DE MICROCONTROLADORES
// Archivo: TWI_SLAVE_HX711
// AUTOR: Chris Quelex
// Sensor: HX711
// HARDWARE: ATMEGA328P
//******************************************************************************************************************

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "I2C/I2C.h"

// Pines HX711
#define HX711_DT   PD3
#define HX711_SCK  PD2

// Ajustes de calibración
#define OFFSET  8000000
#define SCALE   1000

// Dirección I2C
#define SLAVE_ADDR 0x08

// Variables globales
volatile uint8_t peso = 0;  // Valor que se enviará al maestro

// Funciones HX711
void HX711_init(void) {
    DDRD &= ~(1 << HX711_DT);  // DT como entrada
    DDRD |= (1 << HX711_SCK);  // SCK como salida
    PORTD &= ~(1 << HX711_SCK);
}

uint8_t HX711_is_ready(void) {
    return (PIND & (1 << HX711_DT)) == 0;
}

long HX711_read(void) {
    long value = 0;
    while (!HX711_is_ready()); // Bloquea hasta que DOUT = 0

    for (uint8_t i = 0; i < 24; i++) {
        PORTD |= (1 << HX711_SCK);
        _delay_us(1);
        value <<= 1;
        if (PIND & (1 << HX711_DT)) value++;
        PORTD &= ~(1 << HX711_SCK);
        _delay_us(1);
    }

    // Pulso extra para ganancia 128
    PORTD |= (1 << HX711_SCK);
    _delay_us(1);
    PORTD &= ~(1 << HX711_SCK);
    _delay_us(1);

    // Convertir a 32 bits con signo
    if (value & 0x800000) value |= 0xFF000000;
    return value;
}

// Inicializar TWI como esclavo
void I2C_Slave_Init(uint8_t address) {
    TWAR = (address << 1);        // Dirección del esclavo
    TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWEA); // Habilitar TWI, interrupción y ACK
}

// ISR TWI
ISR(TWI_vect) {
    uint8_t estado = TWSR & 0xFC;

    switch (estado) {
        case 0x60: // SLA+W recibido
        case 0x70: // SLA+W repetido
            TWCR |= (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWEA);
            break;

        case 0x80: // Data recibido
        case 0x90: // Data recibido repetido
            // Podemos leer un comando si quieres (opcional)
            (void)TWDR; // leer para limpiar TWINT
            TWCR |= (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWEA);
            break;

        case 0xA8: // SLA+R recibido
        case 0xB8: // Data ACK recibido por maestro
            TWDR = peso; // Enviar el valor actualizado
            TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWIE) | (1 << TWEA);
            break;

        default: // Otros estados: liberar bus
            TWCR |= (1 << TWINT) | (1 << TWSTO) | (1 << TWEN) | (1 << TWIE);
            break;
    }
}

// Main
int main(void) {
    HX711_init();
    I2C_Slave_Init(SLAVE_ADDR);
    sei(); // habilitar interrupciones globales

    while (1) {
        // Leer HX711 y actualizar variable global
        long lectura = HX711_read();
        long temp = (lectura - OFFSET) / SCALE;
        if (temp < 0) temp = 0;
        if (temp > 255) temp = 255;
        peso = (uint8_t)temp;
        // Nota: No hay delay largo, la ISR I2C puede responder en cualquier momento
    }
}
