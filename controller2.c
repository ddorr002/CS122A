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


enum States{Init, Joystick, Autonomous, Laser}State;
unsigned char stateVar;
unsigned char joystickVal = 0x00;
void Tick() {

	unsigned char B0 = ~PINB & 0x01; // Transition to Joystick
	unsigned char B1 = ~PINB & 0x02; // Transition to Autonomous
	unsigned char B2 = ~PINB & 0x04; // Transition to Laser
	unsigned char B3 = ~PINB & 0x08; // Laser left
	unsigned char B4 = ~PINB & 0x10; // Laser right

	unsigned short input = ADC;

	switch(State) {
		case Init:
			stateVar = 0x00;
			PORTC = stateVar;
			break;
		case Joystick:
			//Left or Right
			Set_A2D_Pin(0x00);
			vTaskDelay(3);
			input = ADC;
			if(input < 212) {
				joystickVal = 0x0B;
			}
			else if(input > 812) {
				joystickVal = 0x0C;
			}
			else {
				Set_A2D_Pin(0x01);
				vTaskDelay(3);
				input = ADC;
				if(input < 212) {
					joystickVal = 0x0D;
				}
				else if(input > 812) {
					joystickVal = 0x0A;
				}
				else {
					joystickVal = 0x0E;
				}
			}
			if(USART_IsSendReady(0)) {
				USART_Send(joystickVal, 0);
				PORTC = joystickVal;
			}
			break;
		case Autonomous:
			stateVar = 0x02;
			PORTC = stateVar;
			if(USART_IsSendReady(0)) {
				USART_Send(stateVar, 0);
			}
			break;
		case Laser:
			if(B3) {
				stateVar = 0x08;
				PORTC = stateVar;
				if(USART_IsSendReady(0)) {
					USART_Send(stateVar, 0);
				}
			}
			else if(B4) {
				stateVar = 0x10;
				PORTC = stateVar;
				if(USART_IsSendReady(0)) {
					USART_Send(stateVar, 0);
				}
			}
			else {
				stateVar = 0x04;
				PORTC = stateVar;
				if(USART_IsSendReady(0)) {
					USART_Send(stateVar, 0);
				}
			}
			break;
		default:
			break;
	}	
	switch(State) {
		case Init:
			stateVar = 0x01;
			if(USART_IsSendReady(0)) {
				USART_Send(stateVar, 0);
			}
			State = Joystick;
			break;
		case Joystick:
			if(!B0 && B1 && !B2) {
				State = Autonomous;
			}
			else if(!B0 && !B1 && B2) {
				State = Laser;
			}
			else {
				State = Joystick;
			}
			break;
		case Autonomous:
			if(B0 && !B1 && !B2) {
				State = Joystick;
			}
			else if(!B0 && !B1 && B2) {
				State = Laser;
			}
			else {
				State = Autonomous;
			}
			break;
		case Laser:
			if(B0 && !B1 && !B2) {
				State = Joystick;
			}
			else if(!B0 && B1 && !B2) {
				State = Autonomous;
			}
			else {
				State = Laser;
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
	DDRB = 0x00; PORTB = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0x00;

	A2D_init();
	initUSART(0);

	//Start Tasks
	StartSecPulse(1);
	//RunSchedular
	vTaskStartScheduler();

	return 0;
	
}

