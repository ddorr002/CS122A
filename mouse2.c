/*	Partner(s) Name & E-mail:
 *	Lab Section: 022
 *	Assignment: Final Project (mouse)
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

enum States{Init, JoyStick, Autonomous, AutonomousRotate, Laser,}State;
int phase[] = {0x01, 0x03, 0x02, 0x06, 0x04, 0x0C, 0x08, 0x09};
int i = 0;
unsigned char counter = 0;
unsigned char stateVar;
void Tick() {

	unsigned short sensorReading = ADC;

	switch(State) {
		case Init:
			//PORTC = 0x01;
			break;
		case JoyStick:
			if(USART_HasReceived(0)) {
				stateVar = USART_Receive(0);
			}
			//PORTC = 0x01;
			if(stateVar == 0x0A) {
				PORTC = 0x1B;
			}
			else if(stateVar == 0x0B) {
				PORTC = 0x2B;
			}
			else if(stateVar == 0x0C) {
				PORTC = 0x1D;
			}
			else if(stateVar == 0x0D) {
				PORTC = 0x2D;
			}
			else if(stateVar == 0x0E) {
				PORTC = 0x00;
			}
			break;
		case Autonomous:
			if(USART_HasReceived(0)) {
				stateVar = USART_Receive(0);
			}
			//PORTC = 0x02;

			if(sensorReading > 256) {
				PORTC = 0x00; //wheels
			}
			else {
				PORTC = 0x1B; //wheels
			}
			break;
		case AutonomousRotate:
			PORTC = 0x2B;
			counter++;
			break;
		case Laser:
			if(USART_HasReceived(0)) {
				stateVar = USART_Receive(0);
			}
			if(stateVar == 0x08) {	// Clockwise
				//PORTC = 0x08;
				if(i == 8) {
					i = 0;
				}
				PORTB = phase[i];
				i++;
			}
			else if(stateVar == 0x10) { // CounterClockwise
				//PORTC = 0x10;
				if(i == -1) {
					i = 7;
				}
				PORTB = phase[i];
				i--;
			}
			break;
		default:
			break;
	}
	switch(State) {
		case Init:
			State = JoyStick;
			break;
		case JoyStick:
			if(stateVar == 0x02) {
				State = Autonomous;
			}
			else if(stateVar == 0x04) {
				State = Laser;
			}
			else {
				State = JoyStick;
			}
			break;
		case Autonomous:
			if(stateVar == 0x01) {
				State = JoyStick;
			}
			else if(stateVar == 0x04) {
				PORTC = 0x00;	// Turn wheels off when transitioning to laser
				State = Laser;
			}
			else if(sensorReading > 256) {
				State = AutonomousRotate;
			}
			else {
				State = Autonomous;
			}
			break;
		case AutonomousRotate:
			if(counter > 100) {
				counter = 0;
				State = Autonomous;
			}
			else {
				State = AutonomousRotate;
			}
			break;
		case Laser:
			if(stateVar == 0x01) {
				State = JoyStick;
			}
			else if(stateVar == 0x02) {
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
		vTaskDelay(3);
	}
}

void StartSecPulse(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(Task, (signed portCHAR *)"Task", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}


int main(void)
{
	DDRB = 0xFF; PORTB = 0x00;
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

