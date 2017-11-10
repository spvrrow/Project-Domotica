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
 *
 * Red led : uno 8 (PB0) ingerold
 * Yellow led : uno 9 (PB1)
 * Green led : uno 10 (PB2) uitgerold
 * Serial output on USB = PD1 = board pin 1
 * F_OSC = 16 MHz & baud rate = 19.200
 */

// All includes
#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16E6
#include <util/delay.h>
#include <avr/sfr_defs.h>
#include "distance.h"
#include "AVR_TTC_scheduler.h"

//Defines
#define UBBRVAL 51

// creation of variables
unsigned int gv_counter; // 16 bit counter value
volatile uint8_t gv_echo; // a flag
char String[]="D,";
char String2[]=" cm";
int schermstatus = 0; //huidige status van het scherm (0 = ingerold, 1= uitgerold)
int afstand = 120; // bovengrens voor het zonnescherm in centimeters
int i = 0;


// Port initialization
void init_ports(void)
{
	DDRD = (1 << 4);
	DDRD = ~(1 << 3);
	DDRB = 0xFF;
	
}

// Initialization of timers
void init_distance() {
	EICRA |= (1 << ISC10);
	EIMSK |= (1 << INT1);
	TCCR0A = 0;
	TCCR0B = (1 << CS01) | (1 << CS00);
	TIMSK0 = (1 << TOIE0);
	DDRD |= (1 << 4);
}

// Start of ultrasonor sensor code
// Echo on D3
// Trig on D4
unsigned int calc_cm(unsigned int counter)
{
	return counter * (64 / 16) / 58;
}

ISR (TIMER0_OVF_vect) {
	gv_counter+= 1<<8;
}
int afstand_meter()
{
		PORTD |= (1 << 4);
		_delay_us(12);
		PORTD &= ~(1 << 4);
}
unsigned int time = 0;
unsigned int distance = 0;

ISR (INT1_vect){
	if (PIND & (1 << 3)) {
		TCNT0 = 0;
		time = 0;
		} 
	else {
		time += TCNT0;
		distance = time * (64 / 16) / 58;
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
		int tmp = calc_cm(time);
		itoa(tmp, buffer, 10);
		USART_putstring(buffer);
		USART_putstring(String2);
		_delay_ms(3000);
	}
	return 0;
}


// Start code voor lampjes
void knipper(){
	while(1){
		PORTB |= _BV(1);
		_delay_ms(200);
		PORTB &= ~(1 << 1); // set output low
		_delay_ms(200);
	}
	}

void leds(){
 if (i == 0){
	 PORTB |= _BV(2);
	 PORTB &= ~(1 << 0);
	 knipper();
	 if (schermstatus == 1){
		 schermstatus = 0;
	 }
 }
 if(i == 1){
	 PORTB &= ~(1 << 2);
	 PORTB |= _BV(0);
	 knipper();
	 if (schermstatus == 0){
		 schermstatus = 1;
	 }
 }
}

int main(void)
{
	init_ports();
	init_distance();
	while(1){

	serial_conn();
	
	}
	return 0;
}