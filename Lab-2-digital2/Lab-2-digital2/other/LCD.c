/*
 * CFile1.c
 *
 * Created: 18/07/2025 08:52:11
 *  Author: Chris Q
 */ 



#include "LCD.h"

void initLCD8bits(void){
	
	DDRB |= 0x0F;		//less significant bits as outputs (0000_1111)
	DDRD |= 0xFC;		//most significant bits as outputs (1111_1100)
	PORTB = 0;
	PORTD = 0;

	_delay_ms(5);
	LCD_CMD(0x38);				// modo 8 bits/ 2 lineas
	
	_delay_ms(5);
	LCD_CMD(0x0C);				// Enciende el display, apaga el cursor
	
	_delay_ms(10);
	LCD_CMD(0x06);				// Incrementa el cursos
	
	_delay_ms(10);
	LCD_CMD(0x01);				// Limpia el diplay
	
	_delay_ms(10);

}

 void LCD_CMD(char a) {
	 // RS = 0; // => RS = 0 // Dato en el puerto lo va a interpretar como comando
	 PORTD &= ~(1 << PORTD2);

	 LCD_Port(a);

	 // EN = 1; // => E = 1
	 PORTD|= (1 << PORTD3);
	 _delay_ms(4);

	 // EN = 0; // => E = 0
	 PORTD &= ~(1 << PORTD3);
 }
 
 
 // Funcion para colocar en el puerto un valor
 void LCD_Port(char a) {	 
	 if(a & (1 << 4)) PORTB |= (1 << PORTB0);
	 else PORTB &= ~(1 << PORTB0);
	 
	 if(a & (1 << 5)) PORTB |= (1 << PORTB1);
	 else PORTB &= ~(1 << PORTB1);
	 
	 if(a & (1 << 6)) PORTB |= (1 << PORTB2);
	 else PORTB &= ~(1 << PORTB2);
	 
	 if(a & (1 << 7)) PORTB |= (1 << PORTB3);
	 else PORTB &= ~(1 << PORTB3);
	 
	 if(a & (1 << 0)) PORTD |= (1 << PORTD4);
	 else PORTD &= ~(1 << PORTD4);
	 
	 if(a & (1 << 1)) PORTD |= (1 << PORTD5);
	 else PORTD &= ~(1 << PORTD5);
	 
	 if(a & (1 << 2)) PORTD |= (1 << PORTD6);
	 else PORTD &= ~(1 << PORTD6);
	 
	 if(a & (1 << 3)) PORTD |= (1 << PORTD7);
	 else PORTD &= ~(1 << PORTD7);
	 
 }
  
 void LCD_Write_Char( char c){
	PORTD |= (1 << PORTD2);			// RS = 1 modo de datos
	LCD_Port(c);					// Envia el caracter completo
	
	PORTD |= (1 << PORTD3);			// E = 1
	_delay_ms(4);
	PORTD &= ~(1 << PORTD3);		// E = 0
 }
 
 void LCD_Write_String(char *str){
	 while (*str){
		 LCD_Write_Char(*str);
		 str++;
	 }
 }
  
  void LCD_Shift_Right(void){
	  LCD_CMD(0x01);
	  LCD_CMD(0x0C);
	  
  }
  
  void LCD_Shift_Left(void){
	  
	  LCD_CMD(0x01);
	  LCD_CMD(0x08);
	  
  }
  
  void LCD_Set_Cursor(char f, char c){
	  char temp = 0x80; // Valor por defecto para evitar advertencias

	  if (f ==1 ){
		  temp = 0x80 + ( c- 1);	//FIla 1
		   }
		  else if (f == 2){
			
			temp = 0xC0 + ( c - 1);		// FIla 2  
		  }else{
			  return;
		  }
		  LCD_CMD(temp);  
  }
  
 void LCD_clear(void){
	 LCD_Set_Cursor(1,1);
	 LCD_Write_String("                ");
	 LCD_Set_Cursor(2,1);
	 LCD_Write_String("                ");
 }