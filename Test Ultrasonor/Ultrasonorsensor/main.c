//
// Talking to ultrasonic sensor HC-SR04 with an ATtiny84, and
// sending distance data using serial communications.
//
// electronut.in
//

#include <avr/io.h>
#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define F_CPU 8000000

//
// BEGIN: serials comms
//

char pinTX = PINB1;
volatile unsigned char currByte = 0;
volatile int sendingData = 0;

// initialize
void init_serial()
{
	// set TX pin as output
	DDRB = (1 << pinTX);

	// set up timer1:
	cli();
	// 16 bit timer
	// Divide by 1
	TCCR1B |= (1<<CS10);
	// Count cycles - (1/9600)*8000000
	OCR1A = 833;
	// Put Timer/Counter1 in CTC mode
	TCCR1B |= 1<<WGM12;
	// set interrupt flag
	TIMSK1 |= 1<<OCIE1A;
	sei();
}

// send a byte
void send_byte(char data)
{
	// set flag
	sendingData = 1;
	// set current data byte
	currByte = data;
	// wait till done
	while(sendingData);
}

// which bit was sent last? (0-10)
// -1 implies none sent
volatile char bitNum = -1;

// 16-bit timer CTC handler - Interrupt Service Routine
ISR(TIM1_COMPA_vect)
{
	// serial data sent as:
	//
	// 9600 8 N 1
	//
	// start(low)-0-1-2-3-4-5-6-7-stop(high)-idle(high)-idle(high)
	// 12 bits sent per packet

	// idle => high
	if(!sendingData) {
		PORTB |= (1 << pinTX);
	}
	else {

		if(bitNum < 0) {
			// start bit - low
			PORTB &= ~(1 << pinTX);
			// set bit number
			bitNum = 0;
		}
		else if(bitNum >= 8) {
			// stop bit - high
			PORTB |= (1 << pinTX);
			// increment
			bitNum++;

			// send 2 idle - high - bits
			// not necessary strictly speaking
			if(bitNum == 10) {
				// done
				sendingData = 0;
				// unset bit number
				bitNum = -1;
			}
		}
		else {
			// data bits:

			// bit num is in [0, 7]

			// extract relevant bit from data
			char dataBit = currByte & (1 << bitNum);
			if(dataBit) {
				PORTB |= (1 << pinTX);
			}
			else {
				PORTB &= ~(1 << pinTX);
			}

			// update bit number
			bitNum++;
		}
	}
}


// write null terminated string
void send_str(const char* str)
{
	int len = strlen(str);
	int i;
	for (i = 0; i < len; i++) {
		send_byte(str[i]);
	}
}

//
// END: serials comms
//


//
// BEGIN: HC-SR04 code
//
// pins for HC-SR04
int pinEcho = PIND0;

// initialize HC-SR04
void initHCSR04()
{
	// initialize HC-SR04 pins
	
	// set Trigger pin (connected to PB0) as output
	DDRB |= (1 << PINB0);
}

// 1/0 flag to check if echo is over
volatile char echoDone = 0;

// current timer0 count
uint32_t countTimer0 = 0;

// get distance in cm from HC-SR04
float getDistanceHCSR04()
{
	// Send a 10us HIGH pulse on the Trigger pin.
	// The sensor sends out a “sonic burst” of 8 cycles.
	// Listen to the Echo pin, and the duration of the next HIGH
	// signal will give you the time taken by the sound to go back
	// and forth from sensor to target.

	float distance = 0.0f;

	// enable pin-change interrupt on PCINT0:
	cli();
	// enable interrupt
	GIMSK |= (1 << PCIE0);
	// enable pin
	PCMSK0 |= (1 << PCINT0);
	sei();

	// set echo flag
	echoDone = 0;
	// reset counter
	countTimer0 = 0;

	// send 10us trigger pulse
	//    _
	// __| |__
	PORTB &= ~(1 << PINB0);
	_delay_us(20);
	PORTB |= (1 << PINB0);
	_delay_us(12);
	PORTB &= ~(1 << PINB0);
	_delay_us(20);

	// listen for echo and time it
	//       ____________
	// _____|            |___

	// loop till echo pin goes low
	while(!echoDone);

	// disable pin-change interrupt:
	// disable interrupt
	GIMSK &= ~(1 << PCIE0);
	// disable pin
	PCMSK0 &= ~(1 << PCINT0);

	// calculate duration
	float duration = countTimer0/8000000.0;

	// dist = duration * speed of sound * 1/2
	// dist in cm = duration in s * 340.26 * 100 * 1/2
	// = 17013*duration
	distance = 17013.0 * duration;
	
	return distance;
}

// timer0 overflow interrupt
ISR(TIM0_OVF_vect)
{
	// increment
	countTimer0 += 255;
}

// pin-change interrupt handler
ISR(PCINT0_vect)
{
	// read PCINT0 (PA0 - pn 13):
	if(PINA & (1 << pinEcho)) {
		// rising edge:

		// start 8-bit timer
		// Divide by 1
		TCCR0B |= (1<<CS00);
		// set overflow interrupt flag
		TIMSK0 |= 1<<TOIE0;

	}
	else {
		// falling edge

		// stop timer
		TCCR0B &= ~(1<<CS00);

		// calculate time passed
		countTimer0 += TCNT0;

		// reset counter in timer0
		TCNT0 = 0;

		// set flag
		echoDone = 1;
	}
}
//
// END: HC-SR04 code
//


//
// main loop:
//
int main (void)
{
	// serial
	init_serial();

	// HC-SR04
	initHCSR04();

	// set as output
	DDRB |= (1 << PB0);

	char str[16];

	float prevDist = 0.0;
	// loop
	while (1) {

		float dist = getDistanceHCSR04();
		// sensor only works till 400 cm - if it exceeds, this value
		// just send previous reading
		if(dist > 500) {
			dist = prevDist;
		}
		// print distance to serial port
		sprintf(str, "%f\n", dist);
		send_str(str);
		prevDist = dist;

		// wait
		_delay_ms(70);
	}
	
	return 1;
}
