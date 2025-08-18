//******************************************************************************************************************
//UNIVERSIDAD DEL VALLE DE GUATEMALA
//IE3054: Electrónica Digital 2
//Archivo: TWI_MASTER
//AUTOR: Chris Q
//TWI_MASTER - Proyecto#1
//HARDWARE: ATMEGA328P
//CREADO: 14/08/2025
//ÚLTIMA MODIFICACIÓN: 18/08/2025
//******************************************************************************************************************

#define F_CPU 16000000UL        // Definir CPU con el que trabajará el microcontrolador
#include <avr/io.h>            
#include <util/delay.h>        
#include <string.h>            
#include <stdio.h>            
#include <avr/interrupt.h>    
#include <stdlib.h>            // Añadido para atof()

// Librerías Propias
#include "I2C/I2C.h"
#include "LCD/LCD.h"
#include "USART/USART.h"

// Direcciones de esclavos
#define slave1 0x08        // Nuevo HX711 (peso)
#define slave2 0x40        // Esclavo Contador (Infrarrojo)
#define LM75_ADDR 0x48     // Dirección del sensor LM75

// Prototipos de función
void setup_leds(void);
void float_to_string(float num, char *buffer, int precision);
void USART_print_number(int num);
void USART_print_numbers_float(float num1, float num2, float num3);
void setupTimer1(void);
void separarValores(char* data);

//******************************************************************************************************************
//VARIABLES GLOBALES
//******************************************************************************************************************
volatile char bufferRX;
volatile char buffer_esp32[50];  

char valor1[10];
char valor2[10];
char valor3_4[20];

uint8_t direccion;
uint8_t temp;            
uint8_t bufferI2C;        
uint8_t valorI2C = 0;    

// Variables de sensores
float peso = 0.0;             // HX711
float Sensor_infrarrojo = 0;    
float temperatura = 0.0;      // Sensor LM75

// Buffers de conversión a texto
char PESO_CHAR[16];            
char ANGULO_CHAR[16];                
char TEMP_CHAR[10];                

//******************************************************************************************************************
//MAIN LOOP
//******************************************************************************************************************
int main(void)
{
    cli();
    I2C_Master_Init(1000,1); // Maestro I2C a 100kHz
    setup_leds();        
    initUART9600();        
    initLCD8bits();        
    setupTimer1();        
    
    // Texto inicial en la LCD
    LCD_Set_Cursor(1, 1);
    LCD_Write_String("Pes:  ");
    LCD_Write_String("Dis:  ");
    LCD_Write_String("Tem: ");
    sei();

    while (1) 
    {
        //----> Lectura del sensor de peso (HX711)
        PORTB |= (1 << PORTB5);  // LED indicador
        I2C_Master_Start();
        bufferI2C = slave1 << 1 & 0b11111110;
        temp = I2C_Master_Write(bufferI2C);
        if (temp != 1) {
            I2C_Master_Stop();
        } else {
            I2C_Master_Write('R');  // comando de lectura
            I2C_Master_Stop();
            _delay_ms(100);
            PORTB &= ~(1 << PORTB5);

            I2C_Master_Start();
            bufferI2C = slave1 << 1 | 0b00000001;
            temp = I2C_Master_Write(bufferI2C);
            if(temp != 1){ I2C_Master_Stop(); }

            TWCR = (1 << TWINT) | (1 << TWEN);
            while (!(TWCR & (1 << TWINT)));

            // Recibir datos del esclavo (modificado para usar I2C_Master_Read)
            char peso_buffer[16] = {0};
            uint8_t index = 0;
            uint8_t data;
            
            // Leer primer byte con ACK
            if (I2C_Master_Read(&data, 1)) {
                peso_buffer[index++] = data;
            }
            
            // Leer bytes adicionales si es necesario
            while (index < 15) {
                if (I2C_Master_Read(&data, 1)) {
                    peso_buffer[index++] = data;
                } else {
                    break;
                }
            }
            
            // Leer último byte con NACK
            if (index > 0) {
                I2C_Master_Read(&data, 0);
                peso_buffer[index++] = data;
            }
            
            peso_buffer[index] = '\0';
            peso = atof(peso_buffer);
            float_to_string(peso, PESO_CHAR, 2);

            // Mostrar en LCD
            LCD_Set_Cursor(1,2);
            LCD_Write_String(PESO_CHAR);
            LCD_Set_Cursor(5,2);
            LCD_Write_String("g");
        }

        //----> Lectura del Sensor Infrarrojo 
        PORTC|=(1 << PORTC0);    
        I2C_Master_Start();
        bufferI2C = slave2 << 1 & 0b11111110;    
        temp = I2C_Master_Write(bufferI2C);
        if(temp != 1){
            I2C_Master_Stop();
        }else{                    
            I2C_Master_Write('C');
            I2C_Master_Stop();
            _delay_ms(100);
            PORTC &= ~(1 << PORTC0);
            
            I2C_Master_Start();
            bufferI2C = slave2 << 1 | 0b00000001;
            temp = I2C_Master_Write(bufferI2C);
            if(temp != 1){ I2C_Master_Stop(); }
            TWCR=(1 << TWINT)|(1 << TWEN);
            while (!(TWCR & (1 << TWINT)));
            valorI2C= TWDR;                        
            I2C_Master_Stop();
            
            Sensor_infrarrojo =  180.0 - ((valorI2C*180.0)/255.0);
            float_to_string(Sensor_infrarrojo, ANGULO_CHAR, 1);        
            
            if (Sensor_infrarrojo <= 99){
                LCD_Set_Cursor(7,2);
                LCD_Write_String(ANGULO_CHAR);
                LCD_Set_Cursor(10,2);
                LCD_Write_String("   ");
            }else{
                LCD_Set_Cursor(7,2);
                LCD_Write_String(ANGULO_CHAR);
                LCD_Set_Cursor(12,2);
                LCD_Write_String(" ");
            }
        }
        
        //----> Lectura del Sensor de Temperatura LM75
        PORTC |= (1 << PORTC1);  // LED indicador
        I2C_Master_Start();
        if (I2C_Master_Write((LM75_ADDR << 1) & 0b11111110) != 1) {
            I2C_Master_Stop();
        } else {
            I2C_Master_Write(0x00);  // Registro temperatura
            I2C_Master_Stop();
            _delay_ms(50);
            
            I2C_Master_Start();
            if (I2C_Master_Write((LM75_ADDR << 1) | 0b00000001) != 1) {
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
                
                LCD_Set_Cursor(13, 2);
                LCD_Write_String(TEMP_CHAR);
                LCD_Set_Cursor(16, 2);
                LCD_Write_String("C");
            }
        }
        PORTC &= ~(1 << PORTC1);  
    }
}

//******************************************************************************************************************
//FUNCIONES
//******************************************************************************************************************
void setup_leds(void){
    DDRB |=(1 << DDB5);            
    PORTB &= ~(1 << PORTB5);
    DDRC |=(1 << DDC0)|(1 << DDC1);            
    PORTC &= ~((1 << PORTC0)|(1 << PORTC1));    
}

void float_to_string(float num, char *buffer, int precision){
    int integer_part = (int)num;
    float decimal_part = num - integer_part;
    char int_buffer[16]; 
    snprintf(int_buffer, sizeof(int_buffer), "%d", integer_part);
    char dec_buffer[16]; 
    for (int i = 0; i < precision; i++) {
        decimal_part *= 10;
    }
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
    OCR1A = 7812; 
    TIMSK1 |= (1 << OCIE1A); 
}

//******************************************************************************************************************
//INTERRUPCIONES
//******************************************************************************************************************
ISR (USART_RX_vect){
    bufferRX = UDR0;
    while(!(UCSR0A &(1<<UDRE0)));        
    UDR0 = bufferRX;
}

ISR(TIMER1_COMPA_vect) {
    writeTextUART(PESO_CHAR);
    writeTextUART(",");
    writeTextUART(ANGULO_CHAR);
    writeTextUART(",");
    writeTextUART(TEMP_CHAR);   // ahora se envía la temperatura
    writeTextUART("\r\n");
}