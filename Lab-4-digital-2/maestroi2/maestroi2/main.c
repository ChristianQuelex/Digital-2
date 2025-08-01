/*
 * maestroi2.c
 *
 * Created: 1/08/2025 10:58:48
 * Author : Chris Q
 */ 


#define F_CPU 16000000	
#include <avr/io.h>		
#include <util/delay.h>	


#include <string.h>			//Proporciona funciones para manipular cadenas de caracteres


#include "I2C/I2C.h"
#include "LCD/LCD.h"

//Se define la dirrecion de los esclavos
#define slave1 0x30		//Direcion del esclavo del ADC
#define slave2 0x40		//Direcion del contador

//Se definen los prototipos de funcion
void setup_leds(void);		//Se define un prefuncion de la conexion de los esclavos

void float_to_string(float num, char *buffer, int precision); //Configura de entero a caracter

//******************************************************************************************************************
//VARIABLES GLOBALES
//******************************************************************************************************************
uint8_t direccion;
uint8_t temp;			//Se define una variable temporal
uint8_t bufferI2C;		//Variable encargada de configurar comunicaci?n: Maestro - Esclavo
uint8_t valorI2C = 0;	//Variable encargada de recibir el dato enviado por el esclavo


//Se establecen las variables del contador y potenciometro
float voltaje_pot = 0;	//Se establece una variable para almacenar el valor del potenciometro recibido
float contador = 0;		//Se establece una variable para almacenar el valor del contador recibido

//Almacenar los valores convertido a texto	
char V_pot[16];			//Variable encargada de almacenar la conversi?n del potenciometro a caracter
char CONT[16];			//Variable encargada de almacenar la conversi?n del contador a caracter

//******************************************************************************************************************
//LOOP
//******************************************************************************************************************

int main(void)
{
	I2C_Master_Init(1000,1); //Se inicializa como maestro Fscl 100Hz, prescaler 1
	setup_leds();		//Se llama a la funcion de leds "Conexion"
	
	initLCD8bits();		//Se inicializa la LCD 16X2
	
	//Se configuran textos iniciales en la LCD
	LCD_Set_Cursor(4, 1);
	LCD_Write_String("     ");
	LCD_Write_String("  S2: ");
    while (1) 
    {
		
		//----> Lectura del Potenciometro 
		PORTB|=(1 << PORTB5);		//Se enciende la LED del ADC
		
		I2C_Master_Start();
		// Escritura
		bufferI2C = slave1 << 1 & 0b11111110;	//Se configura la comunicacion con el Esclavo del ADC

		temp = I2C_Master_Write(bufferI2C);
		if(temp != 1){				//En el dado caso de no entablar conexion, se detiene la comunicacion

			I2C_Master_Stop();

		}else{		//Si se entabla conexion, se puede seguir la comunicacion
			
			I2C_Master_Write('R');
			I2C_Master_Stop();
			
			_delay_ms(500);
			
			//Se apaga la led del ADC para indicar comunicacion (Parpadeara cada vez que entre al ciclo "else")
			PORTB &= ~(1 << PORTB5);		
			I2C_Master_Start();
			// Lectura
			bufferI2C = slave1 << 1 | 0b00000001;
			
			temp = I2C_Master_Write(bufferI2C);
			if(temp != 1){		
				I2C_Master_Stop();
			}

			TWCR=(1 << TWINT)|(1 << TWEN);
			while (!(TWCR & (1 << TWINT)));

			valorI2C= TWDR;							//Se almacena el valor de la ADC enviado por el esclavo	

			I2C_Master_Stop();
			
			voltaje_pot = (valorI2C*5)/255;		//Se convierte el valor de 255 del ADC a 5V
			float_to_string(voltaje_pot, V_pot, 2);	//Se convierte el valor entero a caracter

			LCD_Set_Cursor(3,2);
			LCD_Write_String(V_pot);				//Se envia el valor del caracter a la LCD
			
			LCD_Set_Cursor(7,2);
			LCD_Write_Char('V');
			
		}
		
		
		
		//----> Lectura del Contador
		
		PORTC|=(1 << PORTC0);	//Se enciende la LED del Contador (Permanecera encendida si no hay comunicacion)
		
		I2C_Master_Start();
		// Escritura
		bufferI2C = slave2 << 1 & 0b11111110;	//Se configura la comunicacion con el Esclavo del Contador

		temp = I2C_Master_Write(bufferI2C);
		if(temp != 1){				//En el dado caso de no entablar conexion, se detiene la comunicacion

			I2C_Master_Stop();

			}else{					//Si se entabla conexion, se puede seguir la comunicacion
			
			I2C_Master_Write('C');
			I2C_Master_Stop();
			
			_delay_ms(500);

			//Se apaga la led del contador indicacomunicacion (Parpadeara cada vez que entre al ciclo "else")
			PORTC &= ~(1 << PORTC0);
			I2C_Master_Start();
			// Lectura
			bufferI2C = slave2 << 1 | 0b00000001;
			
			temp = I2C_Master_Write(bufferI2C);
			if(temp != 1){
				I2C_Master_Stop();
			}

			TWCR=(1 << TWINT)|(1 << TWEN);
			while (!(TWCR & (1 << TWINT)));

			valorI2C= TWDR;						//Se almacena el valor del contador enviado por el esclavo

			I2C_Master_Stop();
			
			contador = valorI2C;				
			float_to_string(contador, CONT, 2);		//Se transforma el valor del contador a caracter

			LCD_Set_Cursor(11,2);
			LCD_Write_String(CONT);
		
		}

    }
}


//******************************************************************************************************************
//FUNCIONES
//******************************************************************************************************************
void setup_leds(void){
	//Configuraci?n de la Led Esclavo 1 - ADC
	DDRB |=(1 << DDB5);			//Se convigura el PB5 como salida
	PORTB &= ~(1 << PORTB5);	//Se apaga PB5
	
	//Configuraci?n de la Led Esclavo 2 - Contador
	DDRC |=(1 << DDC0);			//Se convigura el PC0 como salida
	PORTC &= ~(1 << PORTC0);	//Se apaga PC0
}

void float_to_string(float num, char *buffer, int precision){
	// Parte entera
	int integer_part = (int)num;
	
	// Parte decimal
	float decimal_part = num - integer_part;
	
	// Convertir la parte entera a cadena
	char int_buffer[16]; // Buffer para la parte entera
	snprintf(int_buffer, sizeof(int_buffer), "%d", integer_part);
	
	// Convertir la parte decimal a cadena
	char dec_buffer[16]; // Buffer para la parte decimal
	for (int i = 0; i < precision; i++) {
		decimal_part *= 10;
	}
	
	snprintf(dec_buffer, sizeof(dec_buffer), "%d", (int)decimal_part);
	
	// Combinar ambas partes en el buffer final
	snprintf(buffer, 32, "%s.%s", int_buffer, dec_buffer);
}

