//******************************************************************************************************************
// UNIVERSIDAD DEL VALLE DE GUATEMALA
// IE2023: PROGRAMACI�N DE MICROCONTROLADORES
// Archivo: TWI_SLAVE_ULTRASONIC
// AUTOR: Chris Quelex y Josu� Castro
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
#include "I2C/I2C.h"     // Tu librer�a original

/* ====== PAR�METROS EDITABLES ====== */
#define THRESHOLD_CM     25     // distancia de activaci�n (cm)
#define SERVO_NEAR_DEG   15     // �ngulo cuando d < THRESHOLD_CM
#define SERVO_FAR_DEG    90     // �ngulo cuando d >= THRESHOLD_CM

#define USE_SERVO_CALIBRATION   1
#define SERVO_MIN_US            100
#define SERVO_MAX_US            2550
/* ================================== */

/* LED estado: A0 (PC0) */
static inline void led_init(void) { DDRC |= (1 << DDC0); PORTC &= ~(1 << PORTC0); }
static inline void led_on(void)   { PORTC |= (1 << PORTC0); }
static inline void led_off(void)  { PORTC &= ~(1 << PORTC0); }

/* === Variables de la librer�a I2C (definidas en I2C.c) ===
 * Las declaramos como extern para usarlas sin modificar la librer�a.
 * - I2C_SlaveData: byte que el maestro leer� (SLA+R)
 * - I2C_LastReceived: �ltimo byte recibido del maestro (SLA+W) � opcional
 */
extern volatile uint8_t I2C_SlaveData;
extern volatile uint8_t I2C_LastReceived;

int main(void)
{
    led_init();

    /* Servo en D9 (OC1A / PB1) */
    servo_init();
#if USE_SERVO_CALIBRATION
    servo_set_calibration(SERVO_MIN_US, SERVO_MAX_US);
#endif
    servo_set_deg(SERVO_FAR_DEG);

    /* Inicializar I�C esclavo con tu direcci�n (aj�stala si tu maestro usa otra) */
    I2C_Slave_Init(0x40);

    while (1) {
        int16_t d = ultrasonic_read_cm_blocking();  // no toca Timer1

        if (d >= 0) {
            led_on();

            if (d < THRESHOLD_CM) {
                servo_set_deg(SERVO_NEAR_DEG);
            } else {
                servo_set_deg(SERVO_FAR_DEG);
            }

            /* Publicar distancia para que el maestro la lea por I2C (SLA+R) */
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
