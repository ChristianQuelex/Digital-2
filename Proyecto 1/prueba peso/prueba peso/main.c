#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Configuración HX711
#define HX711_DOUT     PD2
#define HX711_SCK      PD3
#define HX711_PORT     PORTD
#define HX711_DDR      DDRD
#define HX711_PIN      PIND

// Variables globales
volatile int32_t tare_offset = 0;
volatile float scale_factor = 1.0;
volatile uint8_t cable_invertido = 0; // 0: Blanco=A-, Verde=A+ | 1: Blanco=A+, Verde=A-
char buffer[100];

// Prototipos de funciones
void USART_Init(unsigned int ubrr);
void USART_Flush(void);
unsigned char USART_Receive(void);
void USART_Transmit(unsigned char data);
void USART_SendString(const char *str);
uint8_t USART_GetString(char *buf, uint8_t max_len);
void HX711_Init(void);
int32_t HX711_Read(void);
void HX711_Tare(uint8_t samples);
void HX711_Calibrate(float known_weight);
void HX711_CheckWiring(void);
void print_menu(void);
float stringToFloat(const char *str);
void printFloat(float value, int decimalPlaces);

void USART_Init(unsigned int ubrr) {
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);
}

void USART_Flush(void) {
	unsigned char dummy;
	while (UCSR0A & (1<<RXC0)) dummy = UDR0;
}

unsigned char USART_Receive(void) {
	while (!(UCSR0A & (1<<RXC0)));
	return UDR0;
}

void USART_Transmit(unsigned char data) {
	while (!(UCSR0A & (1<<UDRE0)));
	UDR0 = data;
}

void USART_SendString(const char *str) {
	while (*str) {
		USART_Transmit(*str++);
	}
}

uint8_t USART_GetString(char *buf, uint8_t max_len) {
	uint8_t i = 0;
	char c;
	
	USART_Flush();
	
	while(i < (max_len-1)) {
		c = USART_Receive();
		
		if(c == '\n' || c == '\r') {
			USART_Transmit('\r');
			USART_Transmit('\n');
			break;
		}
		else if(c == '\b' || c == 127) {
			if(i > 0) {
				i--;
				USART_Transmit('\b');
				USART_Transmit(' ');
				USART_Transmit('\b');
			}
		}
		else if(isprint(c)) {
			buf[i++] = c;
			USART_Transmit(c);
		}
	}
	
	buf[i] = '\0';
	return i;
}

void HX711_Init(void) {
	HX711_DDR |= (1 << HX711_SCK);
	HX711_DDR &= ~(1 << HX711_DOUT);
	HX711_PORT &= ~(1 << HX711_SCK);
}

int32_t HX711_Read(void) {
	uint16_t timeout = 10000;
	while ((HX711_PIN & (1 << HX711_DOUT)) && timeout--) _delay_us(1);
	
	if(timeout == 0) {
		USART_SendString("Error: Timeout HX711\r\n");
		return 0;
	}

	int32_t data = 0;
	for (uint8_t i = 0; i < 24; i++) {
		HX711_PORT |= (1 << HX711_SCK);
		_delay_us(1);
		data <<= 1;
		if (HX711_PIN & (1 << HX711_DOUT)) data |= 1;
		HX711_PORT &= ~(1 << HX711_SCK);
		_delay_us(1);
	}

	// Configurar ganancia (Canal A, 128)
	HX711_PORT |= (1 << HX711_SCK);
	_delay_us(1);
	HX711_PORT &= ~(1 << HX711_SCK);
	_delay_us(1);

	// Extender signo (24 bits ? 32 bits)
	if (data & 0x800000) data |= 0xFF000000;
	
	return (cable_invertido) ? -data : data; // Ajuste para cableado invertido
}

void HX711_Tare(uint8_t samples) {
	int64_t sum = 0;
	USART_SendString("Realizando tara...\r\n");
	
	for(uint8_t i = 0; i < samples; i++) {
		sum += HX711_Read();
		_delay_ms(100);
	}
	
	tare_offset = sum / samples;
	sprintf(buffer, "Offset: %ld (muestras: %d)\r\n", tare_offset, samples);
	USART_SendString(buffer);
}

void HX711_Calibrate(float known_weight) {
	if(known_weight <= 0) {
		USART_SendString("Error: Peso debe ser > 0\r\n");
		return;
	}
	
	USART_SendString("Coloca el peso y espera...\r\n");
	_delay_ms(1000);
	
	int32_t raw_value = HX711_Read();
	scale_factor = (float)(raw_value - tare_offset) / known_weight;
	
	sprintf(buffer, "DEBUG: RAW=%ld, Offset=%ld, Delta=%ld\r\n",
	raw_value, tare_offset, (raw_value - tare_offset));
	USART_SendString(buffer);
	
	USART_SendString("Calibrado: ");
	printFloat(known_weight, 2);
	USART_SendString("g -> Factor: ");
	printFloat(scale_factor, 2);
	USART_SendString("\r\n");
}

void HX711_CheckWiring(void) {
	USART_SendString("Verificando cableado...\r\n");
	int32_t initial = HX711_Read();
	USART_SendString("Retira todo peso y presiona una tecla...\r\n");
	USART_Receive();
	
	int32_t with_weight = HX711_Read();
	cable_invertido = (with_weight < initial) ? 1 : 0;
	
	sprintf(buffer, "RAW inicial: %ld | Con peso: %ld | Cableado: %s\r\n",
	initial, with_weight,
	cable_invertido ? "INVERTIDO (Blanco=A+)" : "NORMAL (Blanco=A-)");
	USART_SendString(buffer);
}

void print_menu(void) {
	USART_SendString("\r\n=== Menú HX711 ===\r\n");
	USART_SendString("'t': Tara\r\n");
	USART_SendString("'c': Calibrar\r\n");
	USART_SendString("'p': Medir peso\r\n");
	USART_SendString("'w': Verificar cableado\r\n");
	USART_SendString("'m': Menú\r\n> ");
}

float stringToFloat(const char *str) {
	float result = 0.0;
	float factor = 1.0;
	int decimal = 0;
	
	while (*str == ' ') str++;
	
	// Parte entera
	while (*str >= '0' && *str <= '9') {
		result = result * 10.0 + (*str - '0');
		str++;
	}
	
	// Parte decimal
	if (*str == '.') {
		str++;
		while (*str >= '0' && *str <= '9') {
			result = result * 10.0 + (*str - '0');
			factor *= 10.0;
			str++;
		}
	}
	
	return result / factor;
}

void printFloat(float value, int decimalPlaces) {
	if (value < 0) {
		USART_Transmit('-');
		value = -value;
	}
	
	// Parte entera
	int intPart = (int)value;
	sprintf(buffer, "%d.", intPart);
	USART_SendString(buffer);
	
	// Parte decimal
	float decimal = value - intPart;
	for (int i = 0; i < decimalPlaces; i++) {
		decimal *= 10;
		int digit = (int)decimal;
		USART_Transmit('0' + digit);
		decimal -= digit;
	}
}

int main(void) {
	USART_Init(103); // 9600 bauds @ 16MHz
	HX711_Init();
	
	USART_SendString("\r\nSistema HX711 - Listo\r\n");
	print_menu();

	while (1) {
		if (UCSR0A & (1 << RXC0)) {
			char comando = tolower(USART_Receive());
			
			switch (comando) {
				case 't':
				HX711_Tare(10);
				break;
				
				case 'c':
				USART_SendString("Peso conocido (g): ");
				char input[20];
				if(USART_GetString(input, sizeof(input)) > 0) {
					float weight = stringToFloat(input);
					HX711_Calibrate(weight);
				}
				break;
				
				case 'p': {
					int32_t raw = HX711_Read();
					float peso = (scale_factor != 0) ? (raw - tare_offset) / scale_factor : 0;
					USART_SendString("Peso: ");
					printFloat(peso, 2);
					sprintf(buffer, "g (RAW: %ld)\r\n", raw);
					USART_SendString(buffer);
					break;
				}
				
				case 'w':
				HX711_CheckWiring();
				break;
				
				case 'm':
				print_menu();
				break;
				
				default:
				USART_SendString("Comando inválido. 'm' para menú.\r\n");
				break;
			}
			
			USART_SendString("> ");
		}
		_delay_ms(10);
	}
}