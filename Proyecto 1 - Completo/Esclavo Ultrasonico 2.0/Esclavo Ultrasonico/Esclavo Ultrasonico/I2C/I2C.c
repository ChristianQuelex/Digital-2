/*
 * I2C.c
 *
 * Created: 27/02/2025 19:17:58
 *  Author: Chris Q
 */
#include "I2C.h"
#include <avr/interrupt.h>

// ============================================
// Variables globales para modo esclavo
// ============================================
volatile uint8_t I2C_SlaveData    = 0;  // Dato que enviaremos al maestro (SLA+R)
volatile uint8_t I2C_LastReceived = 0;  // Último dato recibido del maestro (SLA+W)

// --------------------------------------------
// Utilidades internas
// --------------------------------------------
static inline void i2c_set_prescaler(uint8_t prescaler_factor)
{
	// Prescaler válido: 1,4,16,64 -> TWSR bits TWPS1:0 = 00,01,10,11
	switch (prescaler_factor) {
		case 1:   TWSR &= ~((1<<TWPS1) | (1<<TWPS0)); break;                // 00
		case 4:   TWSR =  (TWSR & ~((1<<TWPS1)|(1<<TWPS0))) | (1<<TWPS0); break; // 01
		case 16:  TWSR =  (TWSR & ~((1<<TWPS1)|(1<<TWPS0))) | (1<<TWPS1); break; // 10
		case 64:  TWSR |=  (1<<TWPS1) | (1<<TWPS0); break;                       // 11
		default:  // por seguridad, 1
		TWSR &= ~((1<<TWPS1) | (1<<TWPS0));
		break;
	}
}

static inline uint8_t i2c_status(void)
{
	return (TWSR & 0xF8);
}

// ============================================
// Implementación MODO MAESTRO
// ============================================
void I2C_Master_Init(unsigned long SCL_CLOCK, uint8_t Prescaler)
{
	// Prescaler
	i2c_set_prescaler(Prescaler);

	// TWBR = ((F_CPU / SCL) - 16) / (2 * Prescaler)
	unsigned long denom = (unsigned long)(2UL * Prescaler);
	unsigned long twbr_calc = 0;
	if (SCL_CLOCK > 0 && denom > 0) {
		unsigned long temp = (F_CPU / SCL_CLOCK);
		if (temp > 16UL) {
			twbr_calc = (temp - 16UL) / denom;
			} else {
			twbr_calc = 0; // SCL muy alto; fuerza TWBR mínimo
		}
	}
	if (twbr_calc > 255UL) twbr_calc = 255UL;
	TWBR = (uint8_t)twbr_calc;

	// Habilitar TWI
	TWCR = (1<<TWEN);
}

void I2C_Master_Start(void)
{
	// Enviar condición START
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	// Esperar a que se complete
	while (!(TWCR & (1<<TWINT)));
	// Status esperado: 0x08 (START) o 0x10 (repeated START)
	// (No forzamos error si no coincide; el usuario puede revisar con i2c_status() si desea)
}

void I2C_Master_Stop(void)
{
	// Enviar condición STOP
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	// No se espera TWINT aquí; STOP libera el bus
}

uint8_t I2C_Master_Write(uint8_t dato)
{
	TWDR = dato;
	TWCR = (1<<TWINT) | (1<<TWEN);  // escribir dato y esperar
	while (!(TWCR & (1<<TWINT)));

	// Comprobar ACK del esclavo
	// - Si se envió SLA+W y hubo ACK -> 0x18
	// - Si se envió DATO y hubo ACK  -> 0x28
	// En general, si no hay ACK es 0x20/0x30
	uint8_t st = i2c_status();
	if (st == 0x18 || st == 0x28) {
		return 0; // OK (ACK)
		} else {
		return 1; // NACK u otro estado
	}
}

uint8_t I2C_Master_Read(uint8_t *buffer, uint8_t ack)
{
	if (ack) {
		// Leer y responder con ACK (seguir leyendo)
		TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
		} else {
		// Leer y responder con NACK (último byte)
		TWCR = (1<<TWINT) | (1<<TWEN);
	}
	while (!(TWCR & (1<<TWINT)));

	*buffer = TWDR;

	// Estados de recepción:
	// 0x50 -> dato recibido y ACK retornado
	// 0x58 -> dato recibido y NACK retornado
	uint8_t st = i2c_status();
	if (st == 0x50 || st == 0x58) return 0;
	return 1;
}

// ============================================
// Implementación MODO ESCLAVO
// ============================================
void I2C_Slave_Init(uint8_t address)
{
	// Dirección de esclavo (7 bits) en TWAR
	TWAR = (address << 1);

	// Habilitar TWI con ACK y su interrupción
	TWCR = (1<<TWEN) | (1<<TWEA) | (1<<TWIE);

	// Valor por defecto del dato a enviar
	I2C_SlaveData    = 0;
	I2C_LastReceived = 0;

	// Habilitar interrupciones globales
	sei();
}

// ISR de TWI para manejar eventos como esclavo
ISR(TWI_vect)
{
	switch (i2c_status())
	{
		// ---------- SLA+W recibido, ACK ----------
		case 0x60:  // Own SLA+W received; ACK returned
		case 0x68:  // Arbitration lost; own SLA+W received; ACK returned
		case 0x70:  // General call received; ACK returned
		case 0x78:  // Arbitration lost; general call received; ACK returned
		// Preparar a recibir datos del maestro
		TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA) | (1<<TWIE);
		break;

		// ---------- Dato recibido (SLA+W), ACK ----------
		case 0x80:  // Data received; ACK returned (addressed)
		case 0x90:  // Data received; ACK returned (general call)
		I2C_LastReceived = TWDR; // Guardar lo que el maestro envió
		// Seguir aceptando más datos
		TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA) | (1<<TWIE);
		break;

		// ---------- SLA+R recibido, ACK ----------
		case 0xA8:  // Own SLA+R received; ACK returned
		case 0xB0:  // Arbitration lost; own SLA+R received; ACK returned
		TWDR = I2C_SlaveData;  // Cargar el dato a enviar
		// Enviar dato y seguir preparado
		TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA) | (1<<TWIE);
		break;

		// ---------- Dato transmitido, maestro ACK ----------
		case 0xB8:  // Data transmitted; ACK received
		// Puedes mantener el mismo dato, o actualizarlo aquí si quieres streaming
		TWDR = I2C_SlaveData;
		TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA) | (1<<TWIE);
		break;

		// ---------- Dato transmitido, maestro NACK ----------
		case 0xC0:  // Data transmitted; NACK received
		case 0xC8:  // Last data transmitted; ACK received (shouldn't happen often)
		// Listo para próximo ciclo
		TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA) | (1<<TWIE);
		break;

		// ---------- STOP/repeated START recibido ----------
		case 0xA0:  // STOP or repeated START condition received
		// Continuar a la espera de nueva dirección
		TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA) | (1<<TWIE);
		break;

		// ---------- Default: reset flags -}
	}
}