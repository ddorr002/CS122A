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


// Sends signal via bluetooth to the "mouse"
enum States1{Init1, Joystick, Autonomous}State1;
void Tick1() {
	switch(State1) {
		case Init1:
			break;
		case Joystick:
			break;
		case Autonomous:
			break;
		default:
			break;
	}
	switch(State1) {
		case Init1:
			break;
		case Joystick:
			break;
		case Autonomous:
			break;
		default:
			break;
	}	
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

// Drives the LED Matrix, allowing user to see what the joystick is inputting
enum States2{Init2, LEDmatrix}State2; 
void Tick2() {

	unsigned short input = ADC;

	switch(State2) {
		case Init2:
			PORTC = 0x10;
			PORTD = 0xFE;
			break;
		case LEDmatrix:
			Set_A2D_Pin(0x00);
			vTaskDelay(3);
			input = ADC;
			if( input < 500 ) {
				Set_A2D_Pin(0x01);
				vTaskDelay(3);
				input = ADC;
				if(input < 500) {
					//left-down
					if(PORTC != 0x80) {
						PORTC = (PORTC << 1);
					}
					if(PORTD != 0xEF) {
						PORTD = ~PORTD;
						PORTD = (PORTD << 1);
						PORTD = ~PORTD;
					}
				}
				else if(input > 524) {
					//left-up
					if(PORTC != 0x80) {
						PORTC = (PORTC << 1);
					}
					if(PORTD != 0xFE) {
						PORTD = ~PORTD;
						PORTD = (PORTD >> 1);
						PORTD = ~PORTD;
					}
				}
				else {
					//left
					if(PORTC != 0x80) {
						PORTC = (PORTC << 1);
					}
				}
			}
			else if( input > 524 ) {
				Set_A2D_Pin(0x01);
				vTaskDelay(3);
				input = ADC;
				if(input < 500) {
					//right-down
					if( PORTC != 0x01 ) {
						PORTC = (PORTC >> 1);
					}
					if(PORTD != 0xEF) {
						PORTD = ~PORTD;
						PORTD = (PORTD << 1);
						PORTD = ~PORTD;
					}
				}
				else if(input > 524) {
					//right-up
					if(PORTC != 0x01) {
						PORTC = (PORTC >> 1);
					}
					if(PORTD != 0xFE) {
						PORTD = ~PORTD;
						PORTD = (PORTD >> 1);
						PORTD = ~PORTD;
					}
				}
				else {
					//right
					if(PORTC != 0x01) {
						PORTC = (PORTC >> 1);
					}
				}
			}
			else {
				Set_A2D_Pin(0x01);
				vTaskDelay(3);
				input = ADC;
				if(input > 524) {
					//up
					if(PORTD != 0xFE) {
						PORTD = ~PORTD;
						PORTD = (PORTD >> 1);
						PORTD = ~PORTD;
					}
				}
				else if(input < 500) {
					//down
					if(PORTD != 0xEF) {
						PORTD = ~PORTD;
						PORTD = (PORTD << 1);
						PORTD = ~PORTD;
					}
				}
			}
			break;
		default:
			break;
	}
	switch(State2) {
		case Init2:
			State2 = LEDmatrix;
			break;
		case LEDmatrix:
			State2 = LEDmatrix;
			break;
		default:
			State2 = Init2;
			break;
	}
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


void Task() {
	for(;;)
	{
		Tick2();
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

	A2D_init();

	//Start Tasks
	StartSecPulse(1);
	//RunSchedular
	vTaskStartScheduler();

	return 0;
}

