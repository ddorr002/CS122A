/*	Partner(s) Name & E-mail:
 *	Lab Section: 022
 *	Assignment: Final Project
 *	Exercise Description: [optional - include for your own benefit]
 *	
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
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


void A2D_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	// ADEN: Enables analog-to-digital conversion
	// ADSC: Starts analog-to-digital conversion
	// ADATE: Enables auto-triggering, allowing for constant
	//	    analog to digital conversions.
}

void Set_A2D_Pin(unsigned char pinNum) {
	// Pins on PORTA are used as input for A2D conversion
	//    The default channel is 0 (PA0)
	// The value of pinNum determines the pin on PORTA
	//    used for A2D conversion
	// Valid values range between 0 and 7, where the value
	//    represents the desired pin for A2D conversion
	ADMUX = (pinNum <= 0x07) ? pinNum : ADMUX;
	// Allow channel to stabilize
	static unsigned char i = 0;
	for ( i=0; i<15; i++ ) { asm("nop"); }
}


enum States{Init, Joystick, Autonomous}State;
unsigned char data;
void Tick() {
	
	unsigned short sensorReading = ADC; 

	switch(State) {
		case Init:
			break;
		case Joystick:
			if(USART_HasReceived(0)) {
				data = USART_Receive(0);
				if(data == 'w') {
					PORTB = 0x03;
				}
				else if(data == 's') {
					PORTB = 0x05;
				}
				else if(data == 'h') {
					PORTB = 0x00;
				}
			}
			break;
		case Autonomous:
			PORTD = sensorReading;
			if(sensorReading > 256) {
				PORTB = 0x00; //wheels
			}
			else {
				PORTB = 0x03; //wheels
			}
			break;
		default:
			break;
	}	
	switch(State) {
		case Init:
			State = Joystick;
			break;
		case Joystick:
			if(data == '1') {
				State = Autonomous;
			}
			else {
				State = Joystick;
			}
			break;
		case Autonomous:
			if(USART_HasReceived(0)) {
				data = USART_Receive(0);
				if(data == '0') {
					PORTB = 0x00;
					State = Joystick;
				}
			}
			break;
		default:
			State = Init;
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
    //DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

	A2D_init();

	initUSART(0);

	//Start Tasks
	StartSecPulse(1);
	//RunSchedular
	vTaskStartScheduler();

	return 0;
}

