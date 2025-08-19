/*
 * HX711 con ATmega328P - Calibración y medición
 */

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <avr/eeprom.h>

// Pines HX711
#define HX711_DOUT   PD2
#define HX711_SCK    PD3

// EEPROM
uint32_t EEMEM ee_tare_offset;
float    EEMEM ee_scale_factor;

long tare_offset = 0;
float scale_factor = 1.0;

// ================= USART =================
void USART_Init(unsigned int ubrr) {
    UBRR0H = (unsigned char)(ubrr>>8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);
    UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);
}

void USART_Transmit(unsigned char data) {
    while (!(UCSR0A & (1<<UDRE0)));
    UDR0 = data;
}

void USART_SendString(const char* str) {
    while (*str) {
        USART_Transmit(*str++);
    }
}

char USART_Receive(void) {
    while (!(UCSR0A & (1<<RXC0)));
    return UDR0;
}

void USART_ReadString(char* buffer, uint8_t max_len) {
    uint8_t i = 0;
    char c;
    while (i < max_len - 1) {
        c = USART_Receive();
        if (c == '\r' || c == '\n') break;
        buffer[i++] = c;
        USART_Transmit(c);  // eco
    }
    buffer[i] = '\0';
}

void USART_Flush(void) {
    unsigned char dummy;
    while (UCSR0A & (1<<RXC0)) dummy = UDR0;
}

// ================= HX711 =================
void HX711_init(void) {
    DDRD &= ~(1<<HX711_DOUT); // DOUT entrada
    DDRD |=  (1<<HX711_SCK);  // SCK salida
}

long HX711_read(void) {
    while (PIND & (1<<HX711_DOUT));  // esperar listo

    long count = 0;
    for (uint8_t i = 0; i < 24; i++) {
        PORTD |= (1<<HX711_SCK);
        _delay_us(1);
        count = count << 1;
        PORTD &= ~(1<<HX711_SCK);
        _delay_us(1);
        if (PIND & (1<<HX711_DOUT)) count++;
    }

    // pulso extra para modo canal A, ganancia 128
    PORTD |= (1<<HX711_SCK);
    _delay_us(1);
    PORTD &= ~(1<<HX711_SCK);
    _delay_us(1);

    // signo
    if (count & 0x800000) count |= 0xFF000000;
    return count;
}

long HX711_read_average(uint8_t times) {
    long sum = 0;
    for (uint8_t i = 0; i < times; i++) {
        sum += HX711_read();
    }
    return sum / times;
}

// ================= EEPROM =================
void save_calibration(void) {
    eeprom_update_dword(&ee_tare_offset, tare_offset);
    eeprom_update_float(&ee_scale_factor, scale_factor);
}

void load_calibration(void) {
    tare_offset = eeprom_read_dword(&ee_tare_offset);
    scale_factor = eeprom_read_float(&ee_scale_factor);
    if (scale_factor <= 0.0f) scale_factor = 1.0f;
}

// ================= UTIL =================
void printFloat(float val, uint8_t decimals) {
    char buffer[20];
    dtostrf(val, 0, decimals, buffer);  // convierte float a string
    USART_SendString(buffer);
}

// ================= MAIN =================
int main(void) {
    char buffer[20];
    USART_Init(103);   // 9600 baudios con 16MHz
    HX711_init();
    load_calibration();

    USART_SendString("=== Calibracion HX711 ===\r\n");
    USART_SendString("Comandos:\r\n");
    USART_SendString(" t -> Tara\r\n");
    USART_SendString(" c -> Calibrar (peso conocido)\r\n");
    USART_SendString(" p -> Mostrar peso\r\n");

    while (1) {
        char cmd = USART_Receive();

        if (cmd == 't') {
            USART_SendString("\r\nRealizando tara...\r\n");
            tare_offset = HX711_read_average(10);
            save_calibration();
            sprintf(buffer, "Tara guardada en EEPROM: %ld\r\n", tare_offset);
            USART_SendString(buffer);
        }

        else if (cmd == 'c') {
            USART_SendString("\r\nIngresa peso conocido en gramos (ej: 500):\r\n");
            USART_Flush();
            USART_ReadString(buffer, sizeof(buffer));

            USART_SendString("\r\nLeido: ");
            USART_SendString(buffer);
            USART_SendString("\r\n");

            float known_weight = atof(buffer);
            if (known_weight <= 0) {
                USART_SendString("Error: Peso debe ser > 0\r\n");
                continue;
            }

            USART_SendString("Coloca el peso y espera...\r\n");
            long reading = HX711_read_average(10);
            scale_factor = (float)(abs(reading - tare_offset)) / known_weight;

            save_calibration();
            USART_SendString("Calibracion guardada: Factor=");
            printFloat(scale_factor, 4);
            USART_SendString("\r\n");
        }

        else if (cmd == 'p') {
            long reading = HX711_read_average(10);
            float weight = (float)(abs(reading - tare_offset)) / scale_factor;
            USART_SendString("Peso: ");
            printFloat(weight, 2);
            sprintf(buffer, " g (RAW=%ld)\r\n", reading);
            USART_SendString(buffer);
        }
    }
}
