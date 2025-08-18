/*
 * esclavoi2_1.c
 *
 * Created: 1/08/2025 10:59:47
 * Author : Chris Q
 */ 


#define F_CPU 16000000
#include <avr/io.h>		
#include <util/delay.h>	
#include <avr/interrupt.h>	

//Se define variable propia
#define DEBOUNCE_TIME 50  // Tiempo de espera en milisegundos - Para antirrebote


//Se incluyen las Librerias Propias
#include "I2C/I2C.h"

//Se define la dirrecion de la comunicacion (Maesro a Esclavo)
#define SlaveAddress 0x40

//Se definen los prototipos de funcion

void init_setup(void);				//Prototipo de funcion para inicializar los puertos
void refreshPORT(uint8_t valor);	//Prototipo de funcion para las salidas
void Boton_inter(void);				//Prototipo de funcion para las interrupciones (PC0 y PC1)




//VARIABLES GLOBALES

uint8_t buffer = 0;		//Se define la variable que almacenara los "Comandos" enviados por el Maestro
volatile int COUNT=0;	//La variable que servira para setear los leds -> "Se puede usar igualmente 'contacom'
volatile uint8_t contacom = 0;	//Se define la variable que se empleara para el contador


//LOOP


int main(void)
{
	//Se define la salida de la Led de "Conexion"
	DDRB |=(1 << DDB5);		//Se configura PB5 como salida
	PORTB &= ~(1 << PORTB5);	//Se apaga la salida de PB5
	init_setup();	//Se llama a la funci?n de la configuracion de puertos (entradas y salidas)

	I2C_Slave_Init(SlaveAddress);		//Se inicia el esclavo con la direcci?n del maestro designada
	
	Boton_inter();		//Se llama a la configuracion de los botones

	sei();		// Habilitar Interrupciones generales

 
    while (1) 

    {
		// Iniciando secuencia del contador
		contacom = COUNT;
		refreshPORT(COUNT);
		if (buffer=='C')		//Se lee el "Comando" enviado por el maestro
		{
			PINB |= (1<<PINB5);		//La led parpadea si se establecio contacto
			buffer= 0;				//Se setea en 0, y envia el valor del contador
		}

	}
}


//FUNCIONES


void init_setup(void){
	// Configuraci?n de los puertos de Salida
	DDRD |=(1 << DDD2)|(1 << DDD3)|(1 << DDD4)|(1 << DDD5);		//Configuraci?n de las salidas de las LEDS
	refreshPORT(0);
	DDRB |=(1 << DDB5);			//Se configura la LED de conexi?n
	
	//Configuracion de los puertos de Entrada
	DDRC &= ~((1<<PC0)|(1<<PC1));		//Configurar PC0 y PC1 como entras
	PORTC |= (1<<PC0)|(1<<PC1);			// Habilitar resistencias de PULL-UP para PC0 y PC1 

}

void Boton_inter(void){
	//INTERRUPCION PUERTYO C
	PCICR |= (1<<PCIE1);		//PCIE1 -  PCINT[14:8] -> equivale de PC0 a PC6
	
	//HABILITAR  PCINT1
	PCMSK1 |= (1<<PCINT8)|(1<<PCINT9);		//Al trabajar en PCIE1 se debe usar PCMSK18 (Puerto C)
}											//Se habilitan las interrupciones de P0 y PC1


//Se desarrolla la funci?n que emplearan las leds (Contador)
void refreshPORT(uint8_t valor){
	if(valor & 0b00001000){
		PORTD |= (1 << PORTD5);
		}else{
		PORTD &= ~(1 << PORTD5);
	}
	if(valor & 0b00000100){
		PORTD |= (1 << PORTD4);
		}else{
		PORTD &= ~(1 << PORTD4);
	}
	if(valor & 0b00000010){
		PORTD |= (1 << PORTD3);
		}else{
		PORTD &= ~(1 << PORTD3);
	}	if(valor & 0b00000001){
		PORTD |= (1 << PORTD2);
		}else{
		PORTD &= ~(1 << PORTD2);
	}
	
}



//INTERRUPCIONES


ISR(TWI_vect){
	uint8_t estado;
	estado = TWSR & 0xFC;
	switch(estado){
		case 0x60:
		case 0x70:
		TWCR |=(1 << TWINT);
		break;
		case 0x80:
		case 0x90:
		buffer = TWDR;		//Se carga el "Comando" enviado por el Maestro
		TWCR |= (1 << TWINT); // Se limpia la bandera
		break;
		case 0xA8:
		case 0xB8:
		TWDR = contacom; // Cargar el dato
		TWCR = (1 << TWEN)|(1 << TWIE)|(1 << TWINT)|(1 << TWEA); // Inicia el envio
		break;
		default: // Se libera el bus de cualquier error
		TWCR |=(1 << TWINT)|(1 << TWSTO);
		break;
	}
}

	
	
ISR(PCINT1_vect){
	_delay_ms(DEBOUNCE_TIME);

	// Verificacion de estado despues del retardo
	if (!(PINC & (1 << PC0)) && (PINC & (1 << PC1))) {	//Se ejecuta si solamente se presiona PC0
		COUNT++;		//Aumenta el contador
	}

	if (COUNT >= 16) {		//Al llegar a 16, el contador se reinicia a 0
		COUNT = 0;
	}

	if (!(PINC & (1 << PC1)) && (PINC & (1 << PC0))) {		//Se ejecuta si solamente se presiona PC1
		COUNT--;		//Disminuye el contador
	}

	if (COUNT <= 0) {	//Si detecta que la cuenta llega a 0, el valor del contador se setea en 0, 
		COUNT = 0;		
	}

}