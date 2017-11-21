
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "Timer.h"
#include "usart_ATmega1284.h"

unsigned char rotate[] = {0x01, 0x03, 0x02, 0x06, 0x04, 0x0C, 0x08, 0x09};
int next = 0;
unsigned char button1;
unsigned char button2;
int numPhases;
int open;  //0 if closed, 1 if open
unsigned char temp;

void SPI_MasterInit() {
	//make sure that mosi and sck are outout
	DDRB = 0xFF; PORTB = 0x00;
	//Enable SPI, Master, set clock rate fck16
	SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0);
}

void SPI_MasterTransmit(unsigned char cData) {
	//Start Transmission
	SPDR = cData;
	// Wait for transmission complete
	while(!(SPSR & (1<<SPIF)));
}



enum States {init, wait, check1, check2, inorder, inorder1, reverse, reverse1, waitrelease} state;

void tick()
{
	switch(state)
	{
		case init:
		state = wait;
		break;
		case wait:
		if (open == 1 && button1 == 0x01)		//if one of first three buttons pressed, turn counterclockwise.
		{
			state = check2;
		}
		else if (open == 0 && button1 == 0x01)	//if one of second three buttons pressed, turn clockwise.
		{
			state = check1;
		}
		else
		{
			state = wait;		 //else stay in wait state until input is detected.
		}
		break;
		case check1:
		state = inorder;
		break;
		case check2:
		state = reverse;
		break;
		case inorder:
		if (numPhases > 0)
		{
			state = inorder1;
		}
		else
		{
			state = waitrelease;
		}
		break;
		case inorder1:
		if (next >= 7)
		{
			next = -1;
			numPhases--;
			state = inorder;
		}
		else
		{
			next++;
			state = inorder1;
		}
		break;
		case reverse:
		if (numPhases > 0)
		{
			state = reverse1;
		}
		else
		{
			state = waitrelease;
		}
		break;
		case reverse1:
		if (next <= 0)
		{
			next = 8;
			numPhases--;
			state = reverse;
		}
		else
		{
			next--;
			state = reverse1;
		}
		break;
		case waitrelease:			// wait until the button is released before continuing on to next.
		if (!button1)
		{
			if(open == 0){open = 1;}
			else{open = 0;}
			state = wait;
		}
		else
		{
			state = waitrelease;
		}
		break;
		default:
		break;
	}
	switch (state)
	{
		case check1:
		case check2:
		if (button1 == 0x01)
		{
			numPhases = ((90/5.625) * 64)/4;
		}
		else
		{
			numPhases = 0;
		}
		break;
		case inorder1:
		PORTB = 0x00;
		SPI_MasterTransmit(rotate[next]);
		PORTB = 0x01;
		break;
		case reverse1:
		PORTB = 0x00;
		SPI_MasterTransmit(rotate[next]);
		PORTB = 0x01;
		break;
		default:
		break;
	}
}


int main(void)
{
	DDRA = 0xFF; PORTA = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	SPI_MasterInit();
	initUSART(0);
	//TimerOn();
	//TimerSet(25);
	
	open = 0;
	

	
	while(1)
	{
		//PORTC = test;
		
		if(USART_HasReceived(0)){
			temp = USART_Receive(0);
			USART_Flush(0);
		}
		button1 = temp & 0x01;
		button2 = (temp & 0x02)>>1;
		

		
		if(button2){PORTA = 0x01;}
		else{PORTA = 0x00;}
		tick();
		_delay_ms(8);

		//while(!TimerFlag);
		//TimerFlag = 0;
	}
}