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
#define SLAVE_US    0x40    // Esclavo Ultrasonido (1 byte)
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
float   peso_g = 0.0f;       // gramos (0..20000) provenientes del esclavo (2 bytes)
float   distancia = 0.0f;    // cm (0..255)
float   temperatura = 0.0f;  // °C

// Buffers de texto
char PESO_CHAR[16];
char DISTANCIA_CHAR[16];
char TEMP_CHAR[10];

//******************************************************************************************************************
// MAIN
//******************************************************************************************************************
int main(void)
{
    cli();
    I2C_Master_Init(1000,1);   // tu init original
    setup_leds();
    initUART9600();
    initLCD8bits();
    setupTimer1();

    // Etiquetas en LCD (ajusta si lo deseas)
    LCD_Set_Cursor(1, 1);   LCD_Write_String("Pes:");
    LCD_Set_Cursor(7, 1);   LCD_Write_String("Dis:");
    LCD_Set_Cursor(13, 1);  LCD_Write_String("Tem:");
    sei();

    while (1)
    {
        // ====================================================================================
        // PESO (SLAVE_PESO = 0x08)
        //   - Protocolo: el esclavo entrega 2 bytes: MSB y LSB de gramos (0..20000).
        //   - Leemos MSB (ACK) y LSB (NACK), formamos uint16_t y mostramos en la LCD.
        // ====================================================================================
        // (opcional) comando previo 'C', por compatibilidad con tu flujo
        I2C_Master_Start();
        bufferI2C = (SLAVE_PESO << 1) & 0xFE;       // SLA+W
        temp = I2C_Master_Write(bufferI2C);
        if (temp == 1) {
            I2C_Master_Write('C');                  // el esclavo puede ignorarlo
        }
        I2C_Master_Stop();
        _delay_ms(5);

        // Lectura de 2 bytes
        I2C_Master_Start();
        bufferI2C = (SLAVE_PESO << 1) | 0x01;       // SLA+R
        temp = I2C_Master_Write(bufferI2C);
        if (temp != 1) {
            I2C_Master_Stop();
            LCD_Set_Cursor(1, 2);  LCD_Write_String("Err  ");
            LCD_Set_Cursor(6, 2);  LCD_Write_String("g ");
        } else {
            uint8_t msb = 0, lsb = 0;
            I2C_Master_Read(&msb, 1);               // ACK al primer byte
            I2C_Master_Read(&lsb, 0);               // NACK al último byte
            I2C_Master_Stop();

            uint16_t gramos_u16 = ((uint16_t)msb << 8) | lsb;
            peso_g = (float)gramos_u16;            // gramos

            // Mostrar en LCD (ancho 5, ej. "12345")
            snprintf(PESO_CHAR, sizeof(PESO_CHAR), "%5u", (unsigned)gramos_u16);
            LCD_Set_Cursor(1, 2);  LCD_Write_String(PESO_CHAR);
            LCD_Set_Cursor(6, 2);  LCD_Write_String("g");
        }

        // ====================================================================================
        // ULTRASONIDO (SLAVE_US = 0x40) -> 1 byte (cm)
        // ====================================================================================
        PORTC |= (1 << PORTC0); // LED indicador opcional en PC0
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

            // Lectura "cruda" de 1 byte (como ya hacías)
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
        // TEMPERATURA (LM75)
        // ====================================================================================
        PORTC |= (1 << PORTC1); // LED indicador opcional en PC1
        I2C_Master_Start();
        if (I2C_Master_Write((LM75_ADDR << 1) & 0xFE) != 1) {
            I2C_Master_Stop();
        } else {
            I2C_Master_Write(0x00);                 // puntero a registro de temp
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
                raw_temp >>= 5;                     // 11-bit (0.125°C/LSB)
                if (raw_temp & 0x0400) raw_temp |= 0xF800;

                temperatura = raw_temp * 0.125f;
                float_to_string(temperatura, TEMP_CHAR, 1);

                // Control de motor/LED por temperatura (histeresis)
                if (temperatura >= TEMP_THRESHOLD_ON) {
                    MOTOR_PORT |=  (1 << MOTOR_PORT_BIT);
                } else if (temperatura <= TEMP_THRESHOLD_OFF) {
                    MOTOR_PORT &= ~(1 << MOTOR_PORT_BIT);
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
    // PC0/PC1 como indicadores opcionales, PC2 para motor/LED por temperatura
    DDRC |= (1 << DDC0) | (1 << DDC1) | (1 << MOTOR_DDR_BIT);
    PORTC &= ~((1 << PORTC0) | (1 << PORTC1) | (1 << MOTOR_PORT_BIT));
}

void float_to_string(float num, char *buffer, int precision){
    int integer_part = (int)num;
    float decimal_part = num - integer_part;
    char int_buffer[16];
    snprintf(int_buffer, sizeof(int_buffer), "%d", integer_part);
    char dec_buffer[16];
    for (int i = 0; i < precision; i++) decimal_part *= 10.0f;
    snprintf(dec_buffer, sizeof(dec_buffer), "%d", (int)decimal_part);
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
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS12) | (1 << CS10);
    OCR1A = 7812;                  // ?2 Hz
    TIMSK1 |= (1 << OCIE1A);
}

//******************************************************************************************************************
// INTERRUPCIONES
//******************************************************************************************************************
ISR (USART_RX_vect){
    bufferRX = UDR0;
    while(!(UCSR0A &(1<<UDRE0)));
    UDR0 = bufferRX;
}

ISR(TIMER1_COMPA_vect) {
    // Envía CSV por UART: peso(distancia, temperatura)
    // Nota: PESO_CHAR se actualiza en el loop principal; si quieres precisión total,
    // puedes re-formatear aquí con 'peso_g'.
    writeTextUART(PESO_CHAR);
    writeTextUART(",");
    writeTextUART(DISTANCIA_CHAR);
    writeTextUART(",");
    writeTextUART(TEMP_CHAR);
    writeTextUART("\r\n");
}
