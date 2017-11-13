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
#define BAUDRATE 19200
#define BAUD_PRESCALLER (((F_CPU / (BAUDRATE * 16UL))) - 1)

// creation of variables
char String[]="D,";
char String2[]=" cm";
int schermstatus = 0; //huidige status van het scherm (0 = ingerold, 1= uitgerold)
int afstand = 120; // bovengrens voor het zonnescherm in centimeters
int i = 0;

unsigned int time = 0;
unsigned int distance = 0;


// Port initialization
void init_ports(void)
{
		DDRD |= _BV(DDD3);
		DDRD |= _BV(DDD4);
	
}

// Initialization of timers
void init_distance() {
	EICRA |= (1 << ISC10);
	EIMSK |= (1 << INT1);
	TCCR0A = 0;
	TCCR0B = (1 << CS01) | (1 << CS00);
	TIMSK0 = (1 << TOIE0);
}

// Start of ultrasonor sensor code
// Echo on D3
// Trig on D4

ISR (TIMER0_OVF_vect) {
	time+= 1<<8;
}
int afstand_meter()
{
		PORTD |= (1 << 4);
		_delay_us(12);
		PORTD &= ~(1 << 4);
}


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

//initialisatie van uart
void uart_init(void)
{
	UBRR0H = (uint8_t)(BAUD_PRESCALLER>>8);
	UBRR0L = (uint8_t)(BAUD_PRESCALLER);
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	UCSR0C = (3<<UCSZ00);
}

// Functie voor het ontvangen van data
unsigned char USART_receive(void){
	
	while((UCSR0A &(1<<RXC0)) == 0);
	return UDR0;
	
}

//Functie voor het versturen van data
void transmit(unsigned char data)
{
	while(!(UCSR0A & (1<<UDRE0)));
	UDR0 = data;
}
//functie voor het verturen van een string
void USART_putstring(char* StringPtr){
	
	while(*StringPtr != 0x00){ //Here we check if there is still more chars to send, this is done checking the actual char and see if it is different from the null char
		transmit(*StringPtr);//Using the simple send function we send one char at a time
	StringPtr++;}        //We increment the pointer so we can read the next char
	
}
int serial_conn(void){
	
		afstand_meter();
		USART_putstring(String);
		//convert int to string
		char buffer[10];
		itoa(distance, buffer, 10);
		USART_putstring(buffer);
		USART_putstring(String2);
}

int main(void)
{
	uart_init();
	init_distance();
	init_ports();
	while(1){

	serial_conn();
	
	}
	return 0;
}