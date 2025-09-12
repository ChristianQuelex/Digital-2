//******************************************************************************************************************
// UNIVERSIDAD DEL VALLE DE GUATEMALA
// IE2023: PROGRAMACIÓN DE MICROCONTROLADORES
// Archivo: TWI_SLAVE_ULTRASONIC
// AUTOR: Chris Quelex y Josué Castro
// Sensor: Ultrasonido + Servo
// HARDWARE: ATMEGA328P
// CREADO: 17/08/2025
//******************************************************************************************************************

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "ultrasonic/ultrasonic.h"
#include "servo/servo.h"
#include "I2C/I2C.h"

/* ====== PARÁMETROS EDITABLES ====== */
#define THRESHOLD_CM     25
#define SERVO_NEAR_DEG   20    // que se note más el giro al activar override
#define SERVO_FAR_DEG    90

#define USE_SERVO_CALIBRATION   1
#define SERVO_MIN_US            600   // calibración estándar y segura
#define SERVO_MAX_US            2400
/* ================================== */

/* Comandos recibidos por I2C (último byte escrito por el maestro) */
#define SERVO_CMD_ON   0xA1   // override ON  -> forzar SERVO_NEAR_DEG
#define SERVO_CMD_OFF  0xA0   // override OFF -> comportamiento normal

/* LED estado: A0 (PC0) */
static inline void led_init(void) { DDRC |= (1 << DDC0); PORTC &= ~(1 << PORTC0); }
static inline void led_on(void)   { PORTC |= (1 << PORTC0); }
static inline void led_off(void)  { PORTC &= ~(1 << PORTC0); }

/* Variables de la librería I2C (definidas en I2C.c) */
extern volatile uint8_t I2C_SlaveData;    // byte que el maestro leerá (SLA+R)
extern volatile uint8_t I2C_LastReceived; // último byte recibido (SLA+W)

int main(void)
{
	led_init();

	/* Servo en D9 (OC1A / PB1) */
	servo_init();
	#if USE_SERVO_CALIBRATION
	servo_set_calibration(SERVO_MIN_US, SERVO_MAX_US);
	#endif
	servo_set_deg(SERVO_FAR_DEG);

	/* Inicializar I²C esclavo con dirección 0x40 (ajusta si usas otra) */
	I2C_Slave_Init(0x40);

	uint8_t override_active = 0;

	while (1) {
		/* Lee si hay override de servo desde el maestro */
		if (I2C_LastReceived == SERVO_CMD_ON) {
			override_active = 1;
			} else if (I2C_LastReceived == SERVO_CMD_OFF) {
			override_active = 0;
		}

		if (override_active) {
			/* Forzar posición NEAR mientras override ON */
			led_on();
			servo_set_deg(SERVO_NEAR_DEG);
			I2C_SlaveData = (uint8_t)THRESHOLD_CM; // valor estable si el maestro lee distancia
			_delay_ms(50);
			led_off();
			_delay_ms(50);
			continue; // saltar control por sensor
		}

		/* Modo normal: control por distancia */
		int16_t d = ultrasonic_read_cm_blocking();  // no usa Timer1 internamente

		if (d >= 0) {
			led_on();

			if (d < THRESHOLD_CM) {
				servo_set_deg(SERVO_NEAR_DEG);
				} else {
				servo_set_deg(SERVO_FAR_DEG);
			}

			uint8_t d8 = (d > 255) ? 255 : (uint8_t)d;
			I2C_SlaveData = d8;

			_delay_ms(30);
			led_off();
			} else {
			led_off();
			servo_set_deg(SERVO_FAR_DEG);
		}

		_delay_ms(70);
	}
}
