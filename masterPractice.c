/*
 * ddorr002_master_practice.c
 *
 * Created: 11/27/2016 9:32:48 PM
 * Author : ddorr_000
 */ 

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/portpins.h>
#include <avr/pgmspace.h>

//FreeRTOS include files
#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"

#include "usart_ATmega1284.h"


enum States{Off, On}State;
void Tick() {

	unsigned char B0 = ~PINB & 0x01;

	switch(State) {
		case Off:
			//if( USART_IsSendReady(0) ) {
			//	USART_Send(0x00, 0);
			//}
			break;
		case On:
			//if(USART_IsSendReady(0) ) {
			//	USART_Send(0x01, 0);
			//}
			break;
		default:
			break;
	}
	switch(State) {
		case Off:
			if(B0) {
				if(USART_IsSendReady(0) ) {
					USART_Send(0x01, 0);
				}
				State = On;
			}
			else {
				State = Off;
			}
			break;
		case On:
			if(!B0) {
				if( USART_IsSendReady(0) ) {
					USART_Send(0x00, 0);
				}
				State = Off;
			}
			else {
				State = On;
			}
			break;
		default:
			State = Off;
			break;
	}
}


void Task() {
	for(;;)
	{
		Tick();
		vTaskDelay(50);
	}
}

void StartSecPulse(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(Task, (signed portCHAR *)"Task", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}

int main(void)
{
	DDRB = 0x00; PORTB = 0xFF;
	DDRD = 0x00;

	initUSART(0);

	 //Start Tasks
	 StartSecPulse(1);
	 //RunSchedular
	 vTaskStartScheduler();
	 
	 return 0;
}

