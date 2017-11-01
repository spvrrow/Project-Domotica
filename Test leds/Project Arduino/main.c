/*
 * Project Arduino.c
 *
 * Created: 25-Oct-17 14:27:50
 * Author : Arneldvdv
 */ 

#include <avr/io.h>
#include <avr/delay.h>

//blink every half second
#define BLINK_DELAY 500
// switch every 10 seconds
#define UP_DOWN_DELAY 100000
// stop blinking after 5 seconds
#define Blink_CD 5000

int main(void)
{
	//define ports
	DDRB |= _BV(DDB0);
	DDRB |= _BV(DDB1);
	DDRB |= _BV(DDB2);
	DDRD &= _BV(DDD4);
	DDRD &= _BV(DDD5);
    
    while (1) 
    {
		PORTB |= _BV(PORTB0);	
		_delay_ms(UP_DOWN_DELAY);
		PORTB &= ~ _BV(PORTB0);
		PORTB |= _BV(PORTB2);
		_delay_ms(UP_DOWN_DELAY);
		PORTB &= ~ _BV(PORTB2);		
		
		if (PINB & 0b00000111){
			PORTB |= _BV(PORTB1);
			_delay_ms(BLINK_DELAY);
			
		}
    }
}



