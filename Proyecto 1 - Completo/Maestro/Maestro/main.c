//******************************************************************************************************************
// UNIVERSIDAD DEL VALLE DE GUATEMALA
// IE3054: Electrónica Digital 2
// Archivo: TWI_MASTER
// MAESTRO ATmega328P
//******************************************************************************************************************

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <stdlib.h>

// Librerías Propias
#include "I2C/I2C.h"
#include "LCD/LCD.h"
#include "USART/USART.h"

// Direcciones de esclavos
#define SLAVE_PESO  0x08    // Esclavo Peso (envía 2 bytes = gramos)
#define SLAVE_US    0x40    // Esclavo Ultrasonido + Servo
#define LM75_ADDR   0x48    // Sensor LM75

// --- Control de motor/LED por temperatura en PC2 (A2) ---
#define MOTOR_PORT        PORTC
#define MOTOR_DDR         DDRC
#define MOTOR_PORT_BIT    PC2
#define MOTOR_DDR_BIT     DDC2
#define TEMP_THRESHOLD_ON   30.0f
#define TEMP_THRESHOLD_OFF  29.5f

// Prototipos
void setup_leds(void);
void float_to_string(float num, char *buffer, int precision);
void USART_print_number(int num);
void USART_print_numbers_float(float num1, float num2, float num3);
void setupTimer1(void);
void separarValores(char* data);

//******************************************************************************************************************
// VARIABLES GLOBALES
//******************************************************************************************************************
volatile char bufferRX;
volatile char buffer_esp32[50];

char valor1[10];
char valor2[10];
char valor3_4[20];

uint8_t temp;
uint8_t bufferI2C;
uint8_t valorI2C = 0;

// Variables de sensores
float   peso_g = 0.0f;
float   distancia = 0.0f;
float   temperatura = 0.0f;

// Buffers de texto
char PESO_CHAR[16];
char DISTANCIA_CHAR[16];
char TEMP_CHAR[10];

// --- Override remoto por Adafruit IO via ESP32 (!DC:1/0) ---
volatile uint8_t remote_dc_override = 0;
volatile char    rx_line[8];
volatile uint8_t rx_idx = 0;

// --- NUEVO: Override remoto SERVO (!SV:1/0) ---
volatile uint8_t remote_sv_state = 0;           // 0=OFF(far), 1=ON(near)
volatile uint8_t remote_sv_update_pending = 0;  // 1=hay que mandarlo al esclavo

//******************************************************************************************************************
// MAIN
//******************************************************************************************************************
int main(void)
{
    cli();
    I2C_Master_Init(1000,1);
    setup_leds();

    // ===== UART0 a 115200 bps @16 MHz (U2X=1) =====
    UCSR0A |= (1<<U2X0);
    UBRR0H = 0;
    UBRR0L = 16;  // ~115200 bps, error ~2.1%
    UCSR0B = (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0);
    UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);

    initLCD8bits();
    setupTimer1();
    sei();

    // Etiquetas en LCD
    LCD_Set_Cursor(1, 1);   LCD_Write_String("Pes:");
    LCD_Set_Cursor(7, 1);   LCD_Write_String("Dis:");
    LCD_Set_Cursor(13, 1);  LCD_Write_String("Tem:");

    while (1)
    {
        // ====================================================================================
        // Si hay comando pendiente para el SERVO, enviarlo por I2C al esclavo 0x40 (robusto)
        // 0xA1 = override ON (near), 0xA0 = override OFF (far, comportamiento normal)
        // ====================================================================================
        if (remote_sv_update_pending) {
            uint8_t cmd = remote_sv_state ? 0xA1 : 0xA0;
            uint8_t ok = 0;
            for (uint8_t tries = 0; tries < 3 && !ok; ++tries) {
                I2C_Master_Start();
                bufferI2C = (SLAVE_US << 1) & 0xFE;       // SLA+W
                if (I2C_Master_Write(bufferI2C) == 1) {
                    if (I2C_Master_Write(cmd) == 1) {
                        ok = 1;   // ¡comando entregado!
                    }
                }
                I2C_Master_Stop();
                _delay_ms(3);
            }
            if (ok) {
                remote_sv_update_pending = 0;  // solo limpias si hubo ACK
            }
        }

        // ====================================================================================
        // PESO (SLAVE_PESO = 0x08): lee 2 bytes (MSB/LSB), forma gramos
        // ====================================================================================
        I2C_Master_Start();
        bufferI2C = (SLAVE_PESO << 1) & 0xFE;       // SLA+W
        temp = I2C_Master_Write(bufferI2C);
        if (temp == 1) {
            I2C_Master_Write('C');                  // comando opcional
        }
        I2C_Master_Stop();
        _delay_ms(5);

        I2C_Master_Start();
        bufferI2C = (SLAVE_PESO << 1) | 0x01;       // SLA+R
        temp = I2C_Master_Write(bufferI2C);
        if (temp != 1) {
            I2C_Master_Stop();
            LCD_Set_Cursor(1, 2);  LCD_Write_String("Err  ");
            LCD_Set_Cursor(6, 2);  LCD_Write_String("g ");
        } else {
            uint8_t msb = 0, lsb = 0;
            I2C_Master_Read(&msb, 1);
            I2C_Master_Read(&lsb, 0);
            I2C_Master_Stop();

            uint16_t gramos_u16 = ((uint16_t)msb << 8) | lsb;
            peso_g = (float)gramos_u16;

            snprintf(PESO_CHAR, sizeof(PESO_CHAR), "%5u", (unsigned)gramos_u16);
            LCD_Set_Cursor(1, 2);  LCD_Write_String(PESO_CHAR);
            LCD_Set_Cursor(6, 2);  LCD_Write_String("g");
        }

        // ====================================================================================
        // ULTRASONIDO (SLAVE_US = 0x40) -> 1 byte (cm)
        // ====================================================================================
        PORTC |= (1 << PORTC0);
        I2C_Master_Start();
        bufferI2C = (SLAVE_US << 1) & 0xFE;         // SLA+W
        temp = I2C_Master_Write(bufferI2C);
        if (temp != 1) {
            I2C_Master_Stop();
        } else {
            I2C_Master_Write('C');                  // comando
            I2C_Master_Stop();
            _delay_ms(100);
            PORTC &= ~(1 << PORTC0);

            I2C_Master_Start();
            bufferI2C = (SLAVE_US << 1) | 0x01;     // SLA+R
            temp = I2C_Master_Write(bufferI2C);
            if (temp != 1) { I2C_Master_Stop(); }

            TWCR = (1 << TWINT) | (1 << TWEN);
            while (!(TWCR & (1 << TWINT)));
            valorI2C = TWDR;
            I2C_Master_Stop();

            distancia = (float)valorI2C;
            snprintf(DISTANCIA_CHAR, sizeof(DISTANCIA_CHAR), "%3u", (unsigned)valorI2C);
            LCD_Set_Cursor(7, 2);   LCD_Write_String(DISTANCIA_CHAR);
            LCD_Set_Cursor(10, 2);  LCD_Write_String("cm");
        }

        // ====================================================================================
        // TEMPERATURA (LM75) + control motor (si no hay override DC)
        // ====================================================================================
        PORTC |= (1 << PORTC1);
        I2C_Master_Start();
        if (I2C_Master_Write((LM75_ADDR << 1) & 0xFE) != 1) {
            I2C_Master_Stop();
        } else {
            I2C_Master_Write(0x00);
            I2C_Master_Stop();
            _delay_ms(50);

            I2C_Master_Start();
            if (I2C_Master_Write((LM75_ADDR << 1) | 0x01) != 1) {
                I2C_Master_Stop();
            } else {
                uint8_t msb, lsb;
                I2C_Master_Read(&msb, 1);
                I2C_Master_Read(&lsb, 0);
                I2C_Master_Stop();

                int16_t raw_temp = (msb << 8) | lsb;
                raw_temp >>= 5;
                if (raw_temp & 0x0400) raw_temp |= 0xF800;

                temperatura = raw_temp * 0.125f;
                float_to_string(temperatura, TEMP_CHAR, 1);

                if (!remote_dc_override) {
                    if (temperatura >= TEMP_THRESHOLD_ON) {
                        MOTOR_PORT |=  (1 << MOTOR_PORT_BIT);
                    } else if (temperatura <= TEMP_THRESHOLD_OFF) {
                        MOTOR_PORT &= ~(1 << MOTOR_PORT_BIT);
                    }
                }

                LCD_Set_Cursor(13, 2); LCD_Write_String(TEMP_CHAR);
                LCD_Set_Cursor(16, 2); LCD_Write_String("C");
            }
        }
        PORTC &= ~(1 << PORTC1);
    }
}

//******************************************************************************************************************
// FUNCIONES
//******************************************************************************************************************
void setup_leds(void){
    DDRC |= (1 << DDC0) | (1 << DDC1) | (1 << MOTOR_DDR_BIT);
    PORTC &= ~((1 << PORTC0) | (1 << PORTC1) | (1 << MOTOR_PORT_BIT));
}

void float_to_string(float num, char *buffer, int precision){
    int integer_part = (int)num;
    float decimal_part = num - integer_part;
    if (decimal_part < 0) decimal_part = -decimal_part;
    for (int i = 0; i < precision; i++) decimal_part *= 10.0f;

    char int_buffer[16], dec_buffer[16];
    snprintf(int_buffer, sizeof(int_buffer), "%d", integer_part);
    snprintf(dec_buffer, sizeof(dec_buffer), "%d", (int)(decimal_part + 0.5f));
    snprintf(buffer, 32, "%s.%s", int_buffer, dec_buffer);
}

void USART_print_number(int num) {
    char buffer[10];
    sprintf(buffer, "%d\r\n", num);
    writeTextUART(buffer);
}

void USART_print_numbers_float(float num1, float num2, float num3) {
    char buffert[40];
    sprintf(buffert, "%.2f,%.2f,%.2f\n", num1, num2, num3);
    writeTextUART(buffert);
}

void separarValores(char* data) {
    char* token = strtok(data, ";");
    if (token != NULL) { strcpy(valor1, token); }
    token = strtok(NULL, ";");
    if (token != NULL) { strcpy(valor2, token); }
    token = strtok(NULL, ";");
    if (token != NULL) { strcpy(valor3_4, token); }
}

void setupTimer1(void) {
    TCCR1B = 0;
    TCCR1A = 0;
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS12) | (1 << CS10);
    OCR1A = 7812;                 // ~2 Hz a 16 MHz/1024
    TIMSK1 |= (1 << OCIE1A);
}

//******************************************************************************************************************
// INTERRUPCIONES
//******************************************************************************************************************

// UART RX: recibe "!DC:1/0" y "!SV:1/0"
ISR(USART_RX_vect){
    char ch = UDR0;
    if (ch == '\r') return;

    if (ch == '\n') {
        rx_line[rx_idx] = '\0';
        // "!DC:x"
        if (rx_idx >= 5 && rx_line[0]=='!' && rx_line[1]=='D' && rx_line[2]=='C' && rx_line[3]==':') {
            if (rx_line[4]=='1') remote_dc_override = 1;
            else if (rx_line[4]=='0') remote_dc_override = 0;
        }
        // "!SV:x"
        if (rx_idx >= 5 && rx_line[0]=='!' && rx_line[1]=='S' && rx_line[2]=='V' && rx_line[3]==':') {
            if (rx_line[4]=='1') remote_sv_state = 1;
            else if (rx_line[4]=='0') remote_sv_state = 0;
            remote_sv_update_pending = 1;  // enviar al esclavo en el lazo principal
        }

        rx_idx = 0;
        return;
    }

    if (rx_idx < sizeof(rx_line)-1) {
        rx_line[rx_idx++] = ch;
    } else {
        rx_idx = 0;
    }
}

// TIMER1_COMPA: mantiene motor ON si override DC y envía JSON
ISR(TIMER1_COMPA_vect) {
    if (remote_dc_override) {
        MOTOR_PORT |= (1 << MOTOR_PORT_BIT);
    }

    char json_line[80];
    int n = snprintf(json_line, sizeof(json_line),
        "{\"weight_g\": %s, \"dist_cm\": %s, \"temp_c\": %s}\r\n",
        PESO_CHAR, DISTANCIA_CHAR, TEMP_CHAR);

    if (n > 0) {
        writeTextUART(json_line);
    }
}
