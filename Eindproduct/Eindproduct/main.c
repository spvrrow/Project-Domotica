          /*
 * Eindproduct.c
 *
 * Eindproduct voor het project zonneschermen
 *
 * Created: 20-10-2017 14:44:43
 *  Author: ICT-MAX
 */ 

/* 
 * HC-SR04
 * trigger to sensor : uno 4 (PD4) output
 * echo from sensor  : uno 3 (PD3 = INT1) input
 */

/* output on USB = PD1 = board pin 1
*F_OSC = 16 MHz & baud rate = 19.200
*/

// All includes
#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16E6
#include <util/delay.h>
#include <avr/sfr_defs.h>
#include "distance.h"

//Defines
#define UBBRVAL 51

// creation of variables
volatile uint16_t gv_counter; // 16 bit counter value
volatile uint8_t gv_echo; // a flag
char String[]=" The distance is: ";


// Port initialization
void init_ports(void)
{
	DDRD = (1 << 4);
	DDRD = ~(1 << 3);
	
}

// Initialization of timers
// prescaling : max time = 2^16/16E6 = 4.1 ms, 4.1 >> 2.3, so no prescaling required
// normal mode, no prescale, stop timer
void init_timer(void)
{
	TCCR1A = 0;
	TCCR1B = 0;
}

// External interrupts initialization
void init_ext_int(void)
{
	// any change triggers ext interrupt 1
	EICRA = (1 << ISC10);
	EIMSK = (1 << INT1);
}


// Start of ultrasonoor sensor code
uint16_t calc_cm(uint16_t counter)
{
	return (gv_counter / 16)/58;
}

int afstand_meter(void)
{
	sei();
		gv_echo = BEGIN;
		PORTD |= _BV(4);
		_delay_us(12);
		PORTD = 0x00;
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

// Start of Serial Code

void uart_init()
{
	// set the baud rate
	UBRR0H = 0;
	UBRR0L = UBBRVAL;
	// disable U2X mode
	UCSR0A = 0;
	// enable transmitter
	UCSR0B = _BV(TXEN0);
	// set frame format : asynchronous, 8 data bits, 1 stop bit, no parity
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
}
void transmit(uint8_t data)
{
	// wait for an empty transmit buffer
	// UDRE is set when the transmit buffer is empty
	loop_until_bit_is_set(UCSR0A, UDRE0);
	// send the data
	UDR0 = data;
}

void USART_putstring(char* StringPtr){
	
	while(*StringPtr != 0x00){ //Here we check if there is still more chars to send, this is done checking the actual char and see if it is different from the null char
		transmit(*StringPtr);//Using the simple send function we send one char at a time
	StringPtr++;}        //We increment the pointer so we can read the next char
	
}

unsigned char USART_receive(void){
	
	while(!(UCSR0A & (1<<RXC0)));
	return UDR0;
	
}

int serial_conn(void){
	uart_init();
	while (1) {
		afstand_meter();
		USART_putstring(String);
		//convert int to string
		char buffer[8];
		int tmp = calc_cm(gv_counter);
		itoa(tmp, buffer, 10);
		USART_putstring(buffer);
		_delay_ms(3000);
	}
	return 0;
}



int main(void)
{
	while(1){
	init_ports();
	init_timer();
	init_ext_int();
	serial_conn();
	afstand_meter();
	}
	return 0;
}