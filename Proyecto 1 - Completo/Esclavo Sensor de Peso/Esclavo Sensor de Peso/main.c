//******************************************************************************************************************
// UNIVERSIDAD DEL VALLE DE GUATEMALA
// IE3054: Electrónica Digital 2
// Archivo: TWI_MASTER
// AUTOR: Chris Q
// TWI_MASTER - Proyecto#1
// HARDWARE: ATMEGA328P
// CREADO: 14/08/2025
// ÚLTIMA MODIFICACIÓN: 18/08/2025
// MAESTRO
//******************************************************************************************************************

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "hx711/hx711.h"
#include "I2C/i2c_slave.h"

// ================== Parámetros =============
#define STARTUP_SETTLE_MS 800
#define TARE_SAMPLES      32    // muestras para tara
#define AVG_SAMPLES       4     // promedio por lectura
#define SHIFT_COUNTS      8     // desplazar delta para caber en 16-bit (delta>>8)

// ================== Globales ===============
volatile uint16_t out_u16 = 0;   // lo que se envía al maestro (cuentas 16-bit)
volatile long     OFFSET_rt = 0; // tara en cuentas HX711

// --------------- main ----------------------
int main(void) {
	HX711_init();
	I2C_Slave_Init(0x08);  // SLAVE_ADDR hardcodeado
	sei();

	_delay_ms(STARTUP_SETTLE_MS);
	OFFSET_rt = tare_hx711(TARE_SAMPLES);  // fija cero real

	while (1) {
		// Lee y promedia
		long raw   = HX711_read_avg(AVG_SAMPLES);
		long delta = raw - OFFSET_rt;       // delta de cuentas desde la tara

		// Solo nos interesan aumentos de carga; negativos -> 0
		if (delta < 0) delta = 0;

		// Escala "a lo bruto" para 16-bit: delta >> SHIFT_COUNTS
		unsigned long s = ((unsigned long)delta) >> SHIFT_COUNTS;
		if (s > 65535UL) s = 65535UL;

		// Publicación atómica (evita lecturas partidas en ISR)
		cli();
		out_u16 = (uint16_t)s;
		tx_snap = out_u16;  // Actualiza también para I2C
		sei();

		// Sin delays largos (el HX711 ya limita el ritmo)
	}
}