#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/interrupt.h>

// Definición esencial al principio
#define F_CPU 16000000UL

// Definición del tipo i2c_status_t
typedef enum {
	I2C_START = 0x08,
	I2C_REP_START = 0x10,
	I2C_MT_SLA_ACK = 0x18,
	I2C_MT_SLA_NACK = 0x20,
	I2C_MT_DATA_ACK = 0x28,
	I2C_MT_DATA_NACK = 0x30,
	I2C_MR_SLA_ACK = 0x40,
	I2C_MR_SLA_NACK = 0x48,
	I2C_MR_DATA_ACK = 0x50,
	I2C_MR_DATA_NACK = 0x58,
	I2C_ARB_LOST = 0x38,
	I2C_ERROR = 0x00
} i2c_status_t;

// Configuración UART
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

// Dirección I2C del LM75
#define LM75_ADDR 0x48

// Prototipos de funciones
void UART_Init(unsigned int ubrr);
void UART_Transmit(unsigned char data);
void UART_PrintString(const char *str);
void UART_PrintFloat(float value, uint8_t decimalPlaces);
void i2c_init(void);
i2c_status_t i2c_start(uint8_t address, uint8_t rw);
i2c_status_t i2c_write(uint8_t data);
uint8_t i2c_read_ack(void);
uint8_t i2c_read_nack(void);
void i2c_stop(void);
float lm75_read_temp(void);

// Inicialización UART
void UART_Init(unsigned int ubrr) {
	UBRR0H = (unsigned char)(ubrr >> 8);
	UBRR0L = (unsigned char)ubrr;
	UCSR0B = (1 << TXEN0) | (1 << RXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

// Transmitir un carácter por UART
void UART_Transmit(unsigned char data) {
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = data;
}

// Imprimir cadena por UART
void UART_PrintString(const char *str) {
	while (*str) {
		UART_Transmit(*str++);
	}
}

// Imprimir número flotante por UART
void UART_PrintFloat(float value, uint8_t decimalPlaces) {
	if (value < 0) {
		UART_Transmit('-');
		value = -value;
	}
	
	// Parte entera
	int integerPart = (int)value;
	char buffer[10];
	itoa(integerPart, buffer, 10);
	UART_PrintString(buffer);
	
	// Parte decimal
	if (decimalPlaces > 0) {
		UART_Transmit('.');
		float decimalPart = value - integerPart;
		for (uint8_t i = 0; i < decimalPlaces; i++) {
			decimalPart *= 10;
			int digit = (int)decimalPart % 10;
			UART_Transmit('0' + digit);
		}
	}
}

// Inicialización I2C
void i2c_init(void) {
	TWSR = 0x00;
	TWBR = ((F_CPU / 100000UL) - 16) / 2;
	TWCR = (1 << TWEN);
}

// Generar condición START
i2c_status_t i2c_start(uint8_t address, uint8_t rw) {
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
	
	uint8_t status = TWSR & 0xF8;
	if (status != 0x08 && status != 0x10) return status;
	
	TWDR = (address << 1) | rw;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
	
	return TWSR & 0xF8;
}

// Escribir un byte por I2C
i2c_status_t i2c_write(uint8_t data) {
	TWDR = data;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
	return TWSR & 0xF8;
}

// Leer con ACK
uint8_t i2c_read_ack(void) {
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
	while (!(TWCR & (1 << TWINT)));
	return TWDR;
}

// Leer con NACK
uint8_t i2c_read_nack(void) {
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
	return TWDR;
}

// Generar condición STOP
void i2c_stop(void) {
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
	_delay_us(100);
}

// Leer temperatura del LM75
float lm75_read_temp(void) {
	uint8_t data[2];
	
	if(i2c_start(LM75_ADDR, 0) != I2C_MT_SLA_ACK) return -99.9;
	if(i2c_write(0x00) != I2C_MT_DATA_ACK) return -99.9;
	if(i2c_start(LM75_ADDR, 1) != I2C_MR_SLA_ACK) return -99.9;
	
	data[0] = i2c_read_ack();
	data[1] = i2c_read_nack();
	i2c_stop();
	
	int16_t temp = (data[0] << 8) | data[1];
	temp >>= 5;
	if (temp & 0x0400) temp |= 0xF800; // Extender signo para negativo
	
	return temp * 0.125f;
}

int main(void) {
	// Inicializar UART
	UART_Init(MYUBRR);
	
	// Inicializar I2C
	i2c_init();
	
	// Mensaje inicial
	UART_PrintString("Monitor de Temperatura LM75\r\n");
	UART_PrintString("=========================\r\n");
	
	while (1) {
		// Leer temperatura
		float temperatura = lm75_read_temp();
		
		// Mostrar por UART
		UART_PrintString("Temperatura: ");
		UART_PrintFloat(temperatura, 2);
		UART_PrintString(" °C\r\n");
		
		_delay_ms(1000); // Esperar 1 segundo entre lecturas
	}
	
	return 0;
}