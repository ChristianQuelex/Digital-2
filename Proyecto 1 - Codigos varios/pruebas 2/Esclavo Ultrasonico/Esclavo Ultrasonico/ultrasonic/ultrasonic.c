/*
 * ultrasonic.c
 *
 * Created: 16/08/2025 20:41:37
 * Author: Chris Q
 */

#include "ultrasonic.h"
#include <avr/io.h>
#include <util/delay.h>

void ultrasonic_init(void) {
    DDRD |= (1 << TRIG_PIN);   // Trigger como salida
    DDRD &= ~(1 << ECHO_PIN);  // Echo como entrada

    // Inicializar Timer1 para medir microsegundos
    TCCR1A = 0;
    TCCR1B = (1 << CS11); // prescaler = 8
    TCNT1 = 0;
}

uint16_t measure_distance(void) {
    uint32_t sum = 0;
    uint8_t samples = 5; // número de mediciones a promediar

    for(uint8_t i = 0; i < samples; i++){
        // Pulso de trigger
        PORTD &= ~(1 << TRIG_PIN);
        _delay_us(2);
        PORTD |= (1 << TRIG_PIN);
        _delay_us(10);
        PORTD &= ~(1 << TRIG_PIN);

        // Esperar inicio de eco con timeout
        uint32_t timeout = 0;
        while(!(PIND & (1 << ECHO_PIN)) && timeout < 30000){
            timeout++;
        }

        TCNT1 = 0; // Reiniciar Timer1
        timeout = 0;
        while((PIND & (1 << ECHO_PIN)) && timeout < 30000){
            timeout++;
        }

        uint16_t duration = TCNT1;

        // Convertir ticks a distancia en cm
        uint16_t distance_cm = (uint16_t)((duration * 0.5 * 0.0343) / 2);
        sum += distance_cm;

        _delay_ms(10); // Pequeña pausa entre mediciones
    }

    return (uint16_t)(sum / samples); // Promedio
}
