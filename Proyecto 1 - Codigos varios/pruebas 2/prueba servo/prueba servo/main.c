#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "servo/servo.h"
#include "ultrasonic/ultrasonic.h"

#define LED_PIN PC0
#define DISTANCE_THRESHOLD 30

int main(void) {
	DDRC |= (1 << LED_PIN);  // LED salida

	servo_init();
	ultrasonic_init();

	while(1) {
		uint16_t distance = measure_distance();

		if(distance > 0 && distance < DISTANCE_THRESHOLD) {
			PORTC |= (1 << LED_PIN);
			servo_set(SERVO_MAX);
			} else {
			PORTC &= ~(1 << LED_PIN);
			servo_set(SERVO_MIN);
		}

		_delay_ms(100);
	}
}
