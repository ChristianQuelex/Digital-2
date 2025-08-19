//******************************************************************************************************************
// UNIVERSIDAD DEL VALLE DE GUATEMALA
// IE2023: PROGRAMACIÓN DE MICROCONTROLADORES
// Archivo: TWI_SLAVE_ULTRASONIC
// AUTOR: Chris Q
// Sensor: Ultrasonido + Servo
// HARDWARE: ATMEGA328P
// CREADO: 17/08/2025
//******************************************************************************************************************

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>

// Librerías propias
#include "I2C/I2C.h"
#include "servo/servo.h"
#include "ultrasonic/ultrasonic.h"

// Dirección I2C del esclavo
#define SlaveAddress 0x40

// Pines y constantes
#define LED_PIN PC0
#define DISTANCE_THRESHOLD 30  // Umbral en cm para accionar el servo

//******************************************************************************************************************
// VARIABLES GLOBALES
//******************************************************************************************************************
volatile uint8_t buffer = 0;         // Comando recibido del maestro
volatile uint16_t distance = 0;      // Distancia medida por ultrasonido
volatile uint8_t flag_measure = 0;   // Flag para medir distancia cada 100 ms

//******************************************************************************************************************
// PROTOTIPOS
//******************************************************************************************************************
void setup(void);
void setupTimer0(void);

//******************************************************************************************************************
// MAIN LOOP
//******************************************************************************************************************
int main(void) {
    setup();
    servo_init();       // Inicializar servo (Timer1 dedicado)
    ultrasonic_init();  // Inicializar pines del sensor
    I2C_Slave_Init(SlaveAddress);
    setupTimer0();      // Timer0 para activar medición cada 100 ms

    sei();   // Habilitar interrupciones globales

    while(1) {

        // ---------- Medición de distancia ----------
        if(flag_measure){
            flag_measure = 0;   // Limpiar flag

            uint16_t dist = measure_distance(); // Medición completa y estable
            distance = dist; // Guardar para I2C

            // Accionar servo y LED solo si el objeto está cerca
            if(distance > 0 && distance < DISTANCE_THRESHOLD){
                PORTC |= (1 << LED_PIN);    // Encender LED
                servo_set(SERVO_MAX);       // Mover servo
            } else {
                PORTC &= ~(1 << LED_PIN);   // Apagar LED
                servo_set(SERVO_MIN);       // Mantener servo en posición mínima
            }
        }

        // ---------- Comando del maestro ----------
        if(buffer == 'C'){  
            PORTC ^= (1 << LED_PIN);   // Parpadeo LED
            buffer = 0;
        }

    }
}

//******************************************************************************************************************
// FUNCIONES
//******************************************************************************************************************
void setup(void){
    DDRC |= (1 << LED_PIN);     // LED como salida
    PORTC &= ~(1 << LED_PIN);   // Apagar LED
}

// Configuración TIMER0 para 100 ms
void setupTimer0(void){
    TCCR0A = (1 << WGM01);              // Modo CTC
    TCCR0B = (1 << CS02) | (1 << CS00); // Prescaler 1024
    OCR0A = 155;                        // Cada 10 ms
    TIMSK0 = (1 << OCIE0A);             // Habilitar interrupción
}

//******************************************************************************************************************
// INTERRUPCIONES
//******************************************************************************************************************

// ISR I2C - Comunicación con Maestro
ISR(TWI_vect){
    uint8_t estado;
    estado = TWSR & 0xFC;

    switch(estado){
        case 0x60: // SLA+W recibido
        case 0x70:
            TWCR |= (1 << TWINT);
            break;

        case 0x80: // Dato recibido
        case 0x90:
            buffer = TWDR;              // Guardar comando
            TWCR |= (1 << TWINT);
            break;

        case 0xA8: // SLA+R recibido -> enviar datos
        case 0xB8:
        {
            uint8_t scaledDistance;
            if(distance > 255) scaledDistance = 255;
            else scaledDistance = (uint8_t)distance;
            TWDR = scaledDistance;  // Mandar solo un byte
            TWCR = (1 << TWEN)|(1 << TWIE)|(1 << TWINT)|(1 << TWEA);
            break;
        }

        default: // Error
            TWCR |= (1 << TWINT)|(1 << TWSTO);
            break;
    }
}

// Timer0 cada 10 ms: solo activa flag
ISR(TIMER0_COMPA_vect){
    static uint8_t contador = 0;
    contador++;
    if(contador >= 10){ // Cada 100 ms
        contador = 0;
        flag_measure = 1;
    }
}
