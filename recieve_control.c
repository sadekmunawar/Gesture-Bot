  
/*
  Receive data and control the motors
*/

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>

#include "nrf24/nrf24.h"


volatile uint8_t data_array[6] = {0,0,0,0,0};
uint8_t address_Tx[5] = {0xE7,0xE7,0xE7,0xE7,0xE7};
uint8_t address_Rx[5] = {0xD7,0xD7,0xD7,0xD7,0xD7};

volatile int distance = 10;

void SetupRadio(){
    nrf24_init();
    nrf24_config(2,sizeof(data_array)); // Channel and payload size
    nrf24_tx_address(address_Tx);
    nrf24_rx_address(address_Rx);   
}


void Initialize() {
	
	cli();
	
	//************************************************Ultrasonic*************************************
	//input capture pin for echo
	DDRB &= ~(1<<DDB0);
	PORTB |= (1<<PORTB0);
	
	//output for trigger
	DDRD |= (1<<DDD7);
	
	//timer 1 setup
	
	// Timer1 setup; 64 prescaler
	TCCR1B |= (1<<CS10);
	TCCR1B |= (1<<CS11);
	TCCR1B &= ~(1<<CS12);
	
	//timer 1 mode normal
	TCCR1A &= ~(1<<WGM10);
	TCCR1A &= ~(1<<WGM11);
	TCCR1B &= ~(1<<WGM12);
	TCCR1B &= ~(1<<WGM13);
	
	//look for rising edge
	TCCR1B |= (1<<ICES1);
	
	//clear interrupt flag
	TIFR1 |=(1<<ICF1);
	
	//enable input capture
	TIMSK1 |= (1<<ICIE1); 
	
	
	//************************************************Servo*************************************
	
	//set up PD5 as output pin for PWM;
	DDRB |= (1<<DDB3);
	
	// set up timer 2; 1024 prescaler
	TCCR2B |= (1<<CS20);
	TCCR2B |= (1<<CS21);
	TCCR2B |= (1<<CS22);
	
	//timer mode: fast PWM
	TCCR2A |= (1<<WGM20);
	TCCR2A |= (1<<WGM21);
	TCCR2B &= ~(1<<WGM22);
	
	// toggle OC2A on compare match
	TCCR2A |= (1<<COM2A1);
	TCCR2A &= ~(1<<COM2A0);
	
	//duty cycle: start at off 5(left)-->40(right)
	OCR2A = 40;  
	
	//************************************************Wheels*************************************
		
	// left two wheels control
	DDRB |= (1<<DDB5);
	DDRB |= (1<<DDB4);
	
	//IN1 and IN4
	PORTB &= ~(1<<PORTB5);    //b5, b4: 1, 0 reverse; 0 1 forward
	//IN2  and IN3
	PORTB &= ~(1<<PORTB4);
	
	// right two wheels control
	DDRB |= (1<<DDB1); 
	DDRB |= (1<<DDB2);
	
	//IN1 and IN4
	PORTB &= ~(1<<PORTB1);  //b3, b2: 1, 0 reverse; 0, 1 forward   
	
	//IN2  and IN3
	PORTB &= ~(1<<PORTB2);
	
	//set up PD5 as output pin for PWM;
	DDRD |= (1<<DDD5);
	
	// set up timer 0; 8 prescaler
	TCCR0B &= ~(1<<CS00);
	TCCR0B |= (1<<CS01);
	TCCR0B &= ~(1<<CS02);
	
	//timer mode: fast PWM
	TCCR0A |= (1<<WGM00);
	TCCR0A |= (1<<WGM01);
	TCCR0B |= (1<<WGM02);
	
	// Non-inverting mode
	TCCR0A &= ~(1<<COM0B0);
	TCCR0A |= (1<<COM0B1);
	
	//frequency: 8kHz
	OCR0A = 250;
	
	//duty cycle: start at 72%
	OCR0B = 180;
	
	sei();
}

// For ultrasonic sensor

ISR(TIMER1_CAPT_vect) {
	
	// high --> signal recieved from echo
	if ((PINB & (1<<PINB0))) {
		
		//reset timer1 count to prevent overflow
		TCNT1 = 0;
	
	   //look for falling edge
	   TCCR1B &= ~(1<<ICES1);
	   
	} else {
		
		distance = ICR1;
		
		//look for rising edge
		TCCR1B |= (1<<ICES1);
		
	}
}

//************************************************Car Control*************************************

void motorsOff() {
	PORTB &= ~(1<<PORTB5);
	PORTB &= ~(1<<PORTB4);
	
	PORTB &= ~(1<<PORTB1); 
	PORTB &= ~(1<<PORTB2);
}

void moveforward() {
	selectSpeed(1,1,1);
	PORTB |= (1<<PORTB5);
	PORTB &= ~(1<<PORTB4);
	
	PORTB &= ~(1<<PORTB1); 
	PORTB |= (1<<PORTB2);
}

void moveReverse() {
	selectSpeed(1,1,1);
	PORTB &= ~(1<<PORTB5);
	PORTB |= (1<<PORTB4);
	
	PORTB |= (1<<PORTB1); 
	PORTB &= ~(1<<PORTB2);
}

void turnRight() {
	selectSpeed(1,1,1);
	PORTB &= ~(1<<PORTB5);
	PORTB &= ~(1<<PORTB4);
	
	PORTB &= ~(1<<PORTB1); 
	PORTB |= (1<<PORTB2);
}

void turnLeft() {
	
	selectSpeed(1,1,1);
	PORTB |= (1<<PORTB5);
	PORTB &= ~(1<<PORTB4);
	
	PORTB &= ~(1<<PORTB1); 
	PORTB &= ~(1<<PORTB2);
	
	
}

// control the speed of the car
void selectSpeed(uint8_t sp2, uint8_t sp1, uint8_t sp0) {
	uint8_t s = (4 * sp2) + (2 * sp1) + 1;
	if (s == 0) {
		motorsOff();
		} else {
		OCR0B = (11 * s) + 169;
	}
}

int main()
{
	Initialize();
    SetupRadio();
	
    while(1) {  

	// trigger ultrasonic sensor
	_delay_ms(60);
	PORTD |= (1<<PORTD7);
	_delay_us(12);
	PORTD &= ~(1<<PORTD7); 
			       
       if(nrf24_dataReady())
        {
            nrf24_getData(data_array);
			
			//move the servo-controlled arm
			if (data_array[5] == 1) {
				motorsOff();
				if (OCR2A == 40) {
					OCR2A = 5; 
				} else {
					OCR2A = 40; 
				}

			} else if (data_array[1] == 0 && data_array[2] == 0 && data_array[3] == 0 && data_array[4] == 0) {
				motorsOff();
			} else {
			if (data_array[1] == 0) {
				selectSpeed(0, 1, 1);
				if (data_array[0] == 1) {
					// move forward only when something is not too close to the car
					if (distance > 300) {
						moveforward();
					}  else {
						motorsOff();
				}
				} else {
					moveReverse();
				}
			} else {
				if (data_array[2] == 1) {
					turnRight();
				} else {
					turnLeft();
				}
			}
			}
        } 	
    }
}
