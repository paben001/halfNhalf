
#include <avr/io.h>
#include <stdio.h>
#include "timer.h"
#include "io.h"
#include "usart_ATmega1284.h"
//#include "io.c"
//#include "bit.h"

char ispressed;
unsigned char temp;
unsigned char button1;
unsigned char button2;
unsigned char isopen;


/*

enum States { idle, press, unpress } state;
	
button() {
	switch(state){//transitions
		case idle:
		case press:
		case unpress:
		default: break;
		
	}//transitions
	switch(state){//actions
		case idle:
		case press:
		case unpress:
		default: break;
	}//actions
}

*/

void LCD_update(){
	if(isopen == 0x01){
		LCD_DisplayString(1, "Now dispensing..~~~~~~~~~~~~~~~~");
		}
		else{
			LCD_DisplayString(1, "Ready to        dispense!");
			}
	return;
}

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	//DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	
	ispressed = 0;
	isopen = 0;
	
	TimerOn();
	TimerSet(50);
	LCD_init();
	initUSART(1);

	LCD_DisplayString(1, "1: Toggle latch 2: Stir");
    while (1) 
    {
		button1 = ~PINA & 0x01;
		button2 = ~PINA & 0x02;
		
		if(button1){
			if(isopen == 0){isopen = 0x01;}
			else{isopen = 0x00;}
			LCD_update();
			if(USART_IsSendReady(1)){
				USART_Send(button1, 1);
				while (!USART_HasTransmitted(1));
			}
		}
		if(USART_IsSendReady(1)){
			USART_Send(0x00, 1);
			while (!USART_HasTransmitted(1));
		}
		if(button2){
			do {
				LCD_DisplayString(1, "Stirring...");
				if(USART_IsSendReady(1)){
					USART_Send(button2, 1);
					while (!USART_HasTransmitted(1));
				}
				button2 = ~PINA & 0x02;
			} while (button2);
			LCD_update();
		}
		
		while(!TimerFlag);
		TimerFlag = 0;
    }
}

