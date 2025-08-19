//******************************************************************************************************************
// UNIVERSIDAD DEL VALLE DE GUATEMALA
// IE3054: Electrónica Digital 2
// Archivo: HX711_I2C_SLAVE
// AUTOR: Chris Q
// HX711_I2C_SLAVE - Proyecto#1
// HARDWARE: ATMEGA328P
// CREADO: 14/08/2025
// ÚLTIMA MODIFICACIÓN: 18/08/2025
//******************************************************************************************************************

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Configuración HX711
#define HX711_DOUT   PD2
#define HX711_PD_SCK PD3

// Variables I2C
#define SLA_ADDR 0x08
volatile char tx_buffer[16];
volatile uint8_t tx_index = 0;
volatile uint8_t tx_length = 0;
volatile float current_weight = 0;

// Calibración
long tare_offset = 0;
float scale_factor = 1000.0; // Ajustar según calibración

//******************************************************************************************************************
// FUNCIONES HX711
//******************************************************************************************************************
long HX711_read(void){
    long value = 0;
    while(PIND & (1<<HX711_DOUT)); // Esperar listo
    
    for(uint8_t i=0;i<24;i++){
        PORTD |= (1<<HX711_PD_SCK);
        _delay_us(1);
        value = (value << 1) | ((PIND & (1<<HX711_DOUT)) ? 1 : 0);
        PORTD &= ~(1<<HX711_PD_SCK);
        _delay_us(1);
    }
    
    // Pulso extra para ganancia
    PORTD |= (1<<HX711_PD_SCK);
    _delay_us(1);
    PORTD &= ~(1<<HX711_PD_SCK);

    if(value & 0x800000) value |= 0xFF000000; // Extender signo
    return value;
}

float HX711_get_units(uint8_t times){
    long sum = 0;
    for(uint8_t i=0;i<times;i++){
        sum += HX711_read();
        _delay_ms(1);
    }
    return (float)(sum / times - tare_offset) / scale_factor;
}

//******************************************************************************************************************
// CONFIGURACIÓN I2C
//******************************************************************************************************************
void I2C_init(uint8_t address){
    TWAR = (address << 1);        // Dirección esclavo
    TWCR = (1<<TWEN)|(1<<TWEA)|(1<<TWINT)|(1<<TWIE);
}

ISR(TWI_vect){
    switch(TWSR & 0xF8){
        case 0x60: case 0x68: // SLA+W recibido
            TWCR = (1<<TWEN)|(1<<TWEA)|(1<<TWINT)|(1<<TWIE);
            break;

        case 0x80: case 0x90: // Dato recibido
            if(TWDR == 'R'){ // Comando de lectura
                dtostrf(current_weight, 0, 2, (char*)tx_buffer);
                tx_length = strlen((char*)tx_buffer);
            }
            TWCR = (1<<TWEN)|(1<<TWEA)|(1<<TWINT)|(1<<TWIE);
            break;

        case 0xA8: case 0xB0: // SLA+R recibido
            tx_index = 0;
            TWDR = tx_buffer[tx_index++];
            TWCR = (1<<TWEN)|(1<<TWEA)|(1<<TWINT)|(1<<TWIE);
            break;

        case 0xB8: // Transmitir dato
            if(tx_index < tx_length){
                TWDR = tx_buffer[tx_index++];
            } else {
                TWDR = 0; // Fin de transmisión
            }
            TWCR = (1<<TWEN)|(1<<TWEA)|(1<<TWINT)|(1<<TWIE);
            break;

        default: // Otros estados
            TWCR = (1<<TWEN)|(1<<TWEA)|(1<<TWINT)|(1<<TWIE);
            break;
    }
}

//******************************************************************************************************************
// MAIN LOOP
//******************************************************************************************************************
int main(void){
    // Configurar pines HX711
    DDRD &= ~(1<<HX711_DOUT); // Entrada
    DDRD |= (1<<HX711_PD_SCK); // Salida
    
    // Inicializar I2C
    I2C_init(SLA_ADDR);
    sei(); // Habilitar interrupciones
    
    // LED de estado
    DDRB |= (1<<PB0);
    
    // Calibración inicial
    tare_offset = HX711_read();
    
    while(1){
        // Actualizar peso
        current_weight = HX711_get_units(3);
        
        // LED indicador
        PORTB ^= (1<<PB0);
        
        // Espera no bloqueante
        for(uint8_t i=0; i<10; i++){
            _delay_ms(10);
        }
    }
}