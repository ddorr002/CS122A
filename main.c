/*
 * ddorr002_slave_practice.c
 *
 * Created: 11/27/2016 10:01:45 PM
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
unsigned char var = 0;
void Tick() {
	switch(State) {
		case Off:
			PORTD = 0x00;
			PORTC = var;
			break;
		case On:
			PORTD = 0x80;
			PORTC = var;
			break;
		default:
			break;
	}
	switch(State) {
		case Off:
			if(USART_HasReceived(0)) {
				var = USART_Receive(0);
			}
			if(var == 0x00) {
				State = Off;
			}
			else{
				State = On;
			}
			break;
		case On:
			if(USART_HasReceived(0)) {
				var = USART_Receive(0);
			}
			if(var == 0x01) {
				State = On;
			}
			else{
				State = Off;
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
    DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

	initUSART(0);

	//Start Tasks
	StartSecPulse(1);
	//RunSchedular
	vTaskStartScheduler();
	
	return 0;
}

