/*
 * main.c
 *
 * Created: 11/10/2017 2:50:03 AM
 * Author : ICT-MAX
 *
 */ 


/* 
 * HC-SR04
 * trigger to sensor : uno 0 (PD0) output
 * echo from sensor  : uno 3 (PD3 = INT1) input
 * 
 * DIO : uno 8  (PB0) data
 * CLK : uno 9  (PB1) clock
 * STB : uno 10 (PB2) strobe
 *
 * uno 3/4/5 leds groen/geel/rood
 * 
 * uno A0 = Temp
 * uno A1 = Light
 *
 */

// All includes
#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/sfr_defs.h>
#include "distance.h"
#include "AVR_TTC_scheduler.h"

//Defines
#define UBBRVAL 51
#define BAUDRATE 19200
#define BAUD_PRESCALLER (((F_CPU / (BAUDRATE * 16UL))) - 1)

// Aanmaken van vaste variabelen
char String[]="T,";
char String2[]="L,";
char stringopen[]="O";
char stringdicht[]="C";
int lichtboven = 200;
int lichtonder = 100;
int afstandboven = 160;
int afstandonder = 5;
int tempboven = 20;
int temponder = 10;

// aanmaken van variabelen die nodig zijn voor functies
int adc_result0 = 0;
int adc_result1 = 0;
int celcius = 0;
volatile int afstand = 160;
int status = 0;

// Start of scheduler code
// The array of tasks
sTask SCH_tasks_G[SCH_MAX_TASKS];

void SCH_Dispatch_Tasks(void)
{
   unsigned char Index;

   // Dispatches (runs) the next task (if one is ready)
   for(Index = 0; Index < SCH_MAX_TASKS; Index++)
   {
      if((SCH_tasks_G[Index].RunMe > 0) && (SCH_tasks_G[Index].pTask != 0))
      {
         (*SCH_tasks_G[Index].pTask)();  // Run the task
         SCH_tasks_G[Index].RunMe -= 1;   // Reset / reduce RunMe flag

         // Periodic tasks will automatically run again
         // - if this is a 'one shot' task, remove it from the array
         if(SCH_tasks_G[Index].Period == 0)
         {
            SCH_Delete_Task(Index);
         }
      }
   }
}

unsigned char SCH_Add_Task(void (*pFunction)(), const unsigned int DELAY, const unsigned int PERIOD)
{
   unsigned char Index = 0;

   // First find a gap in the array (if there is one)
   while((SCH_tasks_G[Index].pTask != 0) && (Index < SCH_MAX_TASKS))
   {
      Index++;
   }

   // Have we reached the end of the list?   
   if(Index == SCH_MAX_TASKS)
   {
      // Task list is full, return an error code
      return SCH_MAX_TASKS;  
   }

   // If we're here, there is a space in the task array
   SCH_tasks_G[Index].pTask = pFunction;
   SCH_tasks_G[Index].Delay =DELAY;
   SCH_tasks_G[Index].Period = PERIOD;
   SCH_tasks_G[Index].RunMe = 0;

   // return position of task (to allow later deletion)
   return Index;
}

unsigned char SCH_Delete_Task(const unsigned char TASK_INDEX)
{
   // Return_code can be used for error reporting, NOT USED HERE THOUGH!
   unsigned char Return_code = 0;

   SCH_tasks_G[TASK_INDEX].pTask = 0;
   SCH_tasks_G[TASK_INDEX].Delay = 0;
   SCH_tasks_G[TASK_INDEX].Period = 0;
   SCH_tasks_G[TASK_INDEX].RunMe = 0;

   return Return_code;
}

void SCH_Init_T1(void)
{
   unsigned char i;

   for(i = 0; i < SCH_MAX_TASKS; i++)
   {
      SCH_Delete_Task(i);
   }

   // Set up Timer 1
   // Values for 1ms and 10ms ticks are provided for various crystals

   // Hier moet de timer periode worden aangepast ....!
   OCR1A = (uint16_t)625;   		     // 10ms = (256/16.000.000) * 625
   TCCR1B = (1 << CS12) | (1 << WGM12);  // prescale op 64, top counter = value OCR1A (CTC mode)
   TIMSK1 = 1 << OCIE1A;   		     // Timer 1 Output Compare A Match Interrupt Enable
}

void SCH_Start(void)
{
      sei();
}

ISR(TIMER1_COMPA_vect)
{
   unsigned char Index;
   for(Index = 0; Index < SCH_MAX_TASKS; Index++)
   {
      // Check if there is a task at this location
      if(SCH_tasks_G[Index].pTask)
      {
         if(SCH_tasks_G[Index].Delay == 0)
         {
            // The task is due to run, Inc. the 'RunMe' flag
            SCH_tasks_G[Index].RunMe += 1;

            if(SCH_tasks_G[Index].Period)
            {
               // Schedule periodic tasks to run again
               SCH_tasks_G[Index].Delay = SCH_tasks_G[Index].Period;
               SCH_tasks_G[Index].Delay -= 1;
            }
         }
         else
         {
            // Not yet ready to run: just decrement the delay
            SCH_tasks_G[Index].Delay -= 1;
         }
      }
   }
}

// _______________Eind van scheduler code______________________


//__________________Begin van initialisatie code_________________

// Port initialization
void init_ports(void)
{
	// poorten voor lampjes
	DDRD |= _BV(DDD3);	// Groen lampje = uitgerold
	DDRD |= _BV(DDD4);	// Geel lampje = open of dicht
	DDRD |= _BV(DDD5);	// Rood lampje = ingerold
	
}

//initialisatie van ADC voor temp sensor
void init_adc_temp()
{
	//(PC0 = input)
	ADMUX = (1<<REFS0) | (1<<ADLAR);
	// enable the ADC & prescale = 128
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

void init_adc_licht()
{
	//(PC1 = input)
	ADMUX = (1<<REFS0)|(1<<MUX0)|(1<<ADLAR);
	// enable the ADC & prescale = 128
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

// Value opragen van de ADC
uint8_t get_adc_value()
{
	ADCSRA |= (1<<ADSC); // start conversion
	loop_until_bit_is_clear(ADCSRA, ADSC);
	return ADCH; // 8-bit resolution, left adjusted
}


//______________________________Start van code voor Serial_____________________________

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

// MAIN! functie van temperatuursensor
int temperatuursensor(void){
		init_adc_temp();
		USART_putstring(String);
		
		//convert int to string
		adc_result0 = get_adc_value();
		int mv = (adc_result0/1024.0)*5000;
		celcius = mv/10;
		char buffer[10];
		itoa(celcius, buffer, 10);
		USART_putstring(buffer);
		USART_putstring(",\n");
}

// MAIN! functie van lichtsensor
int lichtsensor(void){
		init_adc_licht();
		USART_putstring(String2);
		
		//convert int to string
		adc_result1 = get_adc_value();
		char buffer[10];
		itoa(adc_result1, buffer, 10);
		USART_putstring(buffer);
		USART_putstring(",\n");
}

int lampjes(void)
{
	if ((adc_result1 > lichtboven) && (afstand >= afstandonder))
	{
			PORTD |= _BV(PORTD3);
			PORTD &= ~ _BV(PORTD5);
			afstand = afstand - 10;
			USART_putstring(stringdicht);
			USART_putstring(",\n");
			
	}
	
	if ((adc_result1 < lichtonder) && (afstand <= afstandboven))
	{
		PORTD |= _BV(PORTD5);
		PORTD &= ~ _BV(PORTD3);
		afstand = afstand + 10;
		USART_putstring(stringopen);
		USART_putstring(",\n");

	}
	
	/*if ((adc_result0 > tempboven) && (afstand >= afstandonder))
	{
		PORTD |= _BV(PORTD3);
		PORTD &= ~ _BV(PORTD5);
		afstand = afstand - 10;
	}
	
	if ((adc_result0 < temponder) && (afstand <= afstandboven))
	{
		PORTD |= _BV(PORTD5);
		PORTD &= ~ _BV(PORTD3);
		afstand = afstand + 10;
	}*/
}

int knipperen(){
	if	((afstand > afstandonder) && (afstand < afstandboven)){
		PORTD |= _BV(PORTD4);
		_delay_ms(200);
		PORTD &= ~ _BV(PORTD4);
	}
}

int main() {
	//initialisatie functies
	/*
	* bijvoorbeeld init_ports();
	*
	*/
	init_ports();
	uart_init();
	
	SCH_Init_T1();
	
	// taken uitvoeren en taken die in de scheduler moeten
	// bijvoorbeeld SCH_Add_Task(sensor_start, 0, 50);
	// 50 * 10ms = 500ms = halve seconde
	
	SCH_Add_Task(lampjes, 0, 100);
	SCH_Add_Task(temperatuursensor, 0, 1000);
	SCH_Add_Task(lichtsensor, 0, 1100);
	SCH_Add_Task(knipperen, 0, 50);
	
	//start de scheduler
	SCH_Start();
	
	//constante loop voor het uitvoeren van alle taken
	while(1) {
		
		
		SCH_Dispatch_Tasks();
	}
	return 0;
}


