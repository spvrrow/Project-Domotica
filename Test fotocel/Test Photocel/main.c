/*
 * Test Photocel.c
 *
 * Created: 08-Nov-17 16:14:01
 * Author : Arneldvdv
 */ 

#include <avr/io.h>
#define F_CPU 16E6 // used in _delay_ms, 16 MHz
#include <util/delay.h>#include <avr/interrupt.h>char String[]="D,";

//Defines
#define UBBRVAL 51

int adc_result0 = 0;
int max_licht = 1200;


void init_ports(){
	DDRD |= _BV(DDD2);
	}
	
int main(void)
{
	init_ports();
	init_adc();
	serial_conn();

}


void init_adc()
{
	// ref=Vcc, left adjust the result (8 bit resolution),
	// select channel 0 (PC0 = input)
	ADMUX = (1<<REFS0)|(1<<ADLAR);
	// enable the ADC & prescale = 128
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
	}
	
uint8_t get_adc_value()
{
ADCSRA |= (1<<ADSC); // start conversion
loop_until_bit_is_clear(ADCSRA, ADSC);
return ADCH; // 8-bit resolution, left adjusted
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
	UCSR0B = _BV(RXEN0) | _BV(TXEN0);
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
	
	//while(!(UCSR0A & (1<<RXC0)));
	loop_until_bit_is_set(UCSR0A, RXC0); /* Wait until data exists. */
	return UDR0;
	
	//char uart_getchar(void) {
	//	loop_until_bit_is_set(UCSR0A, RXC0); /* Wait until data exists. */
	//	return UDR0;
	//}
}



int serial_conn(void){
	uart_init();
	while (1) {
		USART_putstring(String);
		
		//convert int to string
		
		adc_result0 = get_adc_value() * 10;
		char buffer[10];
		itoa(adc_result0, buffer, 10);
		USART_putstring(buffer);
		lampjes();
		_delay_ms(1000);
	}
	return 0;
}

void lampjes(){
	while(adc_result0 > max_licht){
		PORTD |= _BV(2);
		_delay_ms(1000);
		PORTD &= ~(_BV(2));
	}
	
}

