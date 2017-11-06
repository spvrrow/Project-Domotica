          /*
 * Eindproduct.c
 *
 * ultrasonoor measures distance between 2 and 70 cm
 *
 * Created: 29-6-2016 14:44:43
 *  Author: Arneld & Antonio Soprano
 */ 

/* 
 * HC-SR04
 * trigger to sensor : uno 4 (PD4) output
 * echo from sensor  : uno 3 (PD3 = INT1) input
 * DIO : uno 8  (PB0) data
 * CLK : uno 9  (PB1) clock
 * STB : uno 10 (PB2) strobe
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16E6
#include <util/delay.h>
#include "distance.h"
#define BAUD	9600
#define BRC		((F_CPU/16/BAUD)-1)

volatile uint16_t gv_counter; // 16 bit counter value
volatile uint8_t gv_echo; // a flag
volatile unsigned char value;  

int main(void)
{
    
	USART_Init();
	sei();
	value = 'A';

	for(;;){
		
		USART_SendByte(value);
		_delay_ms(250);
}

void init_ports(void)
{
	DDRD = (1 << 4);
	DDRD = ~(1 << 3);
	
}
void init_timer(void)
// prescaling : max time = 2^16/16E6 = 4.1 ms, 4.1 >> 2.3, so no prescaling required
// normal mode, no prescale, stop timer
{
	TCCR1A = 0;
	TCCR1B = 0;
}

void init_ext_int(void)
{
	// any change triggers ext interrupt 1
	EICRA = (1 << ISC10);
	EIMSK = (1 << INT1);
}


uint16_t calc_cm(uint16_t counter)
{
	return (gv_counter / 16)/58;
}

int main(void)
{
	init_ports();
	init_timer();
	init_ext_int();
	sei();
	while (1)
	{
		gv_echo = BEGIN;
		PORTD |= _BV(0);
		_delay_us(12);
		PORTD = 0x00;
		_delay_ms(30);
		int tmp = calc_cm(gv_counter);
		_delay_ms(500);
	}
	
	
}

ISR (INT1_vect)
{
	if(gv_echo == BEGIN)
	{
		TCNT1 = 0;
		TCCR1B |= (1 << CS10);
		gv_echo = END;
	}
	else {
		TCCR1B = 0;
		gv_counter = TCNT1;
	}
}

volatile unsigned char value;

ISR (USART_RXC_vect){
	value = UDR0;
	PORTB = ~value;
}

void USART_Init(void){
	
	UBRR0L = BRC;//   UBRR0H = (BRC >> 8);
	UCSR0B = ((1<<TXEN0)|(1<<RXEN0) | (1<<RXCIE0));
}

void USART_SendByte(uint8_t u8Data){

	
	while((UCSR0A &(1<<UDRE0)) == 0);

	UDR0 = u8Data;
}

uint8_t USART_ReceiveByte(){
	while((UCSR0A &(1<<RXC0)) == 0);
	return UDR0;
}

