
#include <avr/io.h>
#include <stdio.h>
#include "timer.h"
#include "io.h"
#include "usart_ATmega1284.h"

unsigned char temp;
unsigned char button1;
unsigned char button2;

unsigned char i_dis;
unsigned char cur_dis;
static unsigned char dis_time = 40; // 20 = 1 second;

unsigned char cur_stir;

enum dis_States { dis_init, dis_wait, dis_send } dis_state;
	
void dispense() {
	switch(dis_state){//transitions
		case dis_init:
			i_dis = 0;
			cur_dis = 0;
			dis_state = dis_wait;
			break;
		case dis_wait:
			if(button1){
				if(USART_IsSendReady(1)){
					USART_Send(button1, 1);
					while (!USART_HasTransmitted(1));
				}
				if(USART_IsSendReady(1)){
					USART_Send(0x00, 1);
					while (!USART_HasTransmitted(1));
				}

				cur_dis = 1;
				LCD_update();
				dis_state = dis_send;
			}
			break;
		case dis_send:
			if(i_dis < dis_time){
				i_dis++;
			}
			else{
				i_dis = 0;
				cur_dis = 0;
				LCD_update();
				dis_state = dis_wait;
			}
			break;
		default: break;
		
	}//transitions

	switch(dis_state){//actions
		case dis_init: break;
		case dis_wait:
			i_dis = 0;
			cur_dis = 0;
			break;
		case dis_send: break;
		default: break;
	
	}//actions
}

enum stir_States { stir_init, stir_wait, stirring } stir_state;
	
void stir() {
	switch(stir_state){//transitions
		case stir_init:
			stir_state = stir_wait;
			cur_stir = 0;
			break;
		case stir_wait:
			if(button2){
				cur_stir = 1;
				LCD_update();
				stir_state = stirring;
			}
			break;
		case stirring:
			if(!button2){
				cur_stir = 0;
				LCD_update();
				stir_state = stir_wait;
				if(USART_IsSendReady(1)){
					USART_Send(0x00, 1);
					while (!USART_HasTransmitted(1));
				}
			}
			break;
		default: break;
		
	}//transitions
	switch(stir_state){//actions
		case stir_init: break;
		case stir_wait:
			cur_stir = 0;
			break;
		case stirring:
			if(USART_IsSendReady(1)){
					USART_Send(button2, 1);
					while (!USART_HasTransmitted(1));
				}
			break;
		default: break;
	}//actions
}

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
	if(cur_stir == 0){
		if(cur_dis == 0){
			LCD_DisplayString(1, "1: Toggle latch 2: Stir");
		}
		else{
			LCD_DisplayString(1, "Now dispensing..");
		}
	}
	else{
		LCD_DisplayString(1, "Stirring...");
	}

	return;
}

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	
	TimerOn();
	TimerSet(50);
	LCD_init();
	initUSART(1);

	dis_state = dis_init;
	stir_state = stir_init;
	
	LCD_DisplayString(1, "1: Toggle latch 2: Stir");

    while (1) 
    {
		button1 = ~PINA & 0x01;
		button2 = ~PINA & 0x02;

		dispense();
		stir();
		
		while(!TimerFlag);
		TimerFlag = 0;
    }
}

