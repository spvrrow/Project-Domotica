#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void Send_signal(void);
void Initialize_external_interrupt (void);
void Initialize_timer0 (void);

unsigned char working;
unsigned char rising_edge;
uint16_t timer_value;
int distance_cm;
uint8_t error;

ISR (TIMER0_OVF_vect)
{
	if(rising_edge==1) //Check if there was echo
	{
		timer_value++;
		/*Check if isnt out of range*/
		if(timer_value > 91)
		{
			working = 0;
			rising_edge = 0;
			error = 1;
		}
	}
}
ISR (INT1_vect)
{
	if(working==1) //Check if echo is high, start timer
	{
		if(rising_edge==0)
		{
			rising_edge=1;
			TCNT0 = 0;
			timer_value = 0;
		}
		else //Check if echo turned low, calculate distance
		{
			rising_edge = 0;
			distance_cm = (timer_value*256 + TCNT0)/58;
			working = 0;
		}
	}
}

int main(void)
{
	/*Initializing interrupts and screen */
	Initialize_external_interrupt();
	Initialize_timer0();
	DDRB |=  (1 << PINB1);
	DDRD &=~ (1 << PIND3);
	DDRD |= (1 << PIND4);
	sei();
	while(1)
	{
		Send_signal(); //Restarting for another conversation
	}
}

void Send_signal()
{
	if(working ==0) //Be sure that conversation is finished
	{
		_delay_ms(50);		//Restart HC-SR04
		PORTD &=~ (1 << PIND4);
		_delay_us(1);
		PORTD |= (1 << PIND4); //Send 10us second pulse
		_delay_us(10);
		PORTD &=~ (1 << PIND4);
		working = 1;	//Be sure that it is ready
		error = 0;		//Clean errors
	}
}

void Initialize_external_interrupt()
{
	MCUCR |= (1 << ISC10); //Any logical change on INT1
	GICR |= (1 << INT1); //Enable INT1
}

void Initialize_timer0()
{
	TCCR0 |= (1 << CS00); //No prescaling
	TCNT0 = 0;			//Reset timer
	TIMSK |= (1 << TOIE0); //Timer overflow interrupt enable
}