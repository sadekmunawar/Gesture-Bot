/*
 * transmitter.c
 * Author : sadek
 */ 

/*
    Send data to the receiver
    The nrf24L01 [CE, CSN, SCK, MOSI, MISO] connects to [PD0, PD1, PD2, PD3, PD4]
*/

#define F_CPU 16000000UL

#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include "nrf24/nrf24.h"


volatile uint8_t data_array[6] = {1,1,1,1, 1, 1};
uint8_t address_Tx[5] = {0xE7,0xE7,0xE7,0xE7,0xE7};
uint8_t address_Rx[5] = {0xD7,0xD7,0xD7,0xD7,0xD7};

void sendBytes(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6){
    data_array[0]=b1;
    data_array[1]=b2;
    data_array[2]=b3;
	data_array[3]=b4;
	data_array[4]=b5;
	data_array[5]=b6;

    nrf24_send(data_array);  
    while(nrf24_isSending()) { /* */ }
}


void Initialize() {
	//***************setup ADC*********************
	
	PRR &= ~(1<<PRADC);
	
	// Vref
	ADMUX |= (1<<REFS0); 
	ADMUX &= ~(1<<REFS1); 
	
	//ADC prescaler 128
	ADCSRA |= (1<<ADPS0);
	ADCSRA |= (1<<ADPS1);
	ADCSRA |= (1<<ADPS2);
	
	//channel selection: ADC0
	ADMUX &= ~(1<<MUX0);
	ADMUX &= ~(1<<MUX1);
	ADMUX &= ~(1<<MUX2);
	ADMUX &= ~(1<<MUX3);
	
	DIDR0 = 0x00;	
	
	//enable ADC
	ADCSRA |= (1<<ADEN);
	
	//start conversion
	ADCSRA |= (1<<ADSC);
}

// read ADC channel
int readADC(uint8_t channel)	
{
	//Pick channel
	if (channel == 0) {
		ADMUX &= ~(1<<MUX0);
		ADMUX &= ~(1<<MUX1);
		ADMUX &= ~(1<<MUX2);
		ADMUX &= ~(1<<MUX3);
	} else if (channel == 1) {
		ADMUX |= (1<<MUX0);
		ADMUX &= ~(1<<MUX1);
		ADMUX &= ~(1<<MUX2);
		ADMUX &= ~(1<<MUX3);		
	} else {
		ADMUX &= ~(1<<MUX0);
		ADMUX |= (1<<MUX1);
		ADMUX &= ~(1<<MUX2);
		ADMUX &= ~(1<<MUX3);
	}
	// Start conversion
	ADCSRA |= (1<<ADSC);	
	// Wait until conversion finishes
	while (!(ADCSRA & (1<<ADIF)));	
	// Clear interrupt flag
	ADCSRA |= (1<<ADIF);	
	return ADC;		
}

int main()
{
	DDRB |= (1<<DDB1);
	PORTB |= (1<<PORTB1);

	Initialize();

    nrf24_init();    
    nrf24_config(2,sizeof(data_array)); // Channel and payload size  
    nrf24_tx_address(address_Rx);
    nrf24_rx_address(address_Tx);    

    while(1)
    {
		// hand 
		int x = readADC(0);
		int y = readADC(1);
		
		// finger
		int x_1 = readADC(2);
		
		// instruction to move the arm
		if (x_1 < 280) {
		sendBytes(0,0,0, 0, 0, 1); _delay_ms(500);
		}
		
		// send other control instructions based on ADC values
		else if (x >= 305 && x <= 335 && y >= 305 && y <= 340) {
			sendBytes(0,0,0, 0, 0, 0);  _delay_ms(300); 
		} else if (y >= 315 && y <= 330) {
			if (x > 330) {
				sendBytes(1, 0, 0, 1, 1, 0);   _delay_ms(300);
			} else {
				sendBytes(0, 0, 0, 1, 1, 0);   _delay_ms(300);
			}
		} else if (x >= 315 && x <= 330) {
			if (y > 330) {
				sendBytes(0, 1, 1, 1, 1, 0);   _delay_ms(300);
			} else {
				sendBytes(0, 1, 0, 0, 0, 0);  _delay_ms(300);
			}
		}
    }
}
