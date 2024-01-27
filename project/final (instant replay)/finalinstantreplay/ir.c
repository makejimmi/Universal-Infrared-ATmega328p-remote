#include "common.h"

volatile uint8_t edge = 0;
volatile uint8_t recorded = 0;
volatile uint16_t pulseCount = 0;
volatile uint8_t pulsing = 0;

/* NRcheck == 0 -> either IR signal received or timeout
 * NRcheck == 1 -> timeout checking
 * NRcheck == 2 -> error code (timeout)
 * NRcheck == 3 -> checking end of signal
 * NRcheck == 4 -> end of signal detected, proceeding to save the IR signal in array
 * NRcheck == 5 -> IR signal too large (exceeded MAX_IR_EDGES)
 * NRcheck == 50 -> part of enabling pulsing of recorded IR signal
*/
volatile int8_t NRcheck = 6;
volatile uint16_t NRcount = 0;

/** @brief Replay an IR command
 * 
 * This function replays a command with the given timings from ir
 * array.
 * 
 * @param ir Pointer to array, where the timings are.
 * @return 0 on success, error code otherwise
 * 
 * TBD: implement error codes
 */
 
 //Enables the timer overflow interrupt
void startTimer0(){ TIMSK0 |= (1<<TOIE0); }

//Disables the timer overflow interrupt
void stopTimer0(){ PORTD &= ~(1<<7); TIMSK0 &= ~(1<<TOIE0);}

void timer0conf(uint8_t mode){
	if(!mode){
		TCCR0B &= ~(1<<CS01);
		TCCR0B &= ~(1<<CS02);
		TCCR0B &= ~(1<<CS00);
		TCCR0B |= (1<<CS01); 						//prescaler 8 -> 0.5us
		TCNT0 = 236;								//Reload value on 236 cause it seemed to work best to get to 13 us per pulse
	}
	if(mode){
		TCCR0B &= ~(1<<CS01);
		TCCR0B &= ~(1<<CS02);
		TCCR0B &= ~(1<<CS00);
		TCCR0B |= (1<<CS02) | (1<<CS00); 			//prescaler 1024 -> 64us
		if(NRcheck == 1){ TCNT0 = 6;}				//Overflow at 16ms
		if(NRcheck == 3){ TCNT0 = 255;}				//Overflow at 4 544us
	}
}

uint8_t ir_play_command(uint16_t * ir){
	timer0conf(0);
	DDRB |= (1<<0); //CHECKING
	PORTB |= (1<<0); //CHECKING
	DDRD |= (1<<7);
	PORTD &= ~(1<<7);
	pulse(ir);
	return 0;
}

void pulse(uint16_t *ir){
	uint8_t q = 0;
	uint8_t NOrep = 0;
	startTimer0();
	NRcheck = 50;
	while(ir[q] != 1){
		if((q % 2) == 1){
			if(!NOrep){
				PORTD &= ~(1<<7);
				PORTB &= ~(1<<0); 	//CHECKING
				pulsing = 1;
				NOrep = 1;
			}
			if(pulseCount >= ir[q]){ q++; pulseCount = 0; }
		}
		if((q % 2) == 0){
			if(NOrep){
				
				PORTD &= ~(1<<7);
				PORTB |= (1<<0);   //CHECKING
				pulsing = 0;
				NOrep = 0;
			}
			if(pulseCount >= ir[q]){ q++; pulseCount = 0; }
		}
	}
	stopTimer0();
}

ISR(TIMER0_OVF_vect){  								//timer0 interrupt for 38kHz pulse and error handling
	if((pulsing) && (NRcheck == 50)){
		TCNT0 = 236;
		PORTD^= (1<<7);
		pulseCount += 13;
	}
	if((!pulsing) && (NRcheck == 50)){
		TCNT0 = 236;
		pulseCount+=13;
	}
	if(NRcheck == 1){								//timer to check timeout
		TCNT0 = 6;
		NRcount++;
		if(NRcount >= 625){							//250 * 64 * 625 = 10.000.000us = 10 seconds
			NRcount = 0;
			NRcheck = 2;
			stopTimer0();
			stopTimer1();
		}
	}
	if(NRcheck == 3){								//timer to check end of signal
		NRcount++;
		if(NRcount >= 71){							//since prescaler = 64; 71 * 64 = 4.544 microseconds
			stopTimer0();
			stopTimer1();
			ir_timings[recorded+1] = 1;
			NRcheck = 4;
		}
	}
}

/** @brief Record an IR command
 * 
 * This function records an IR command to the given uint16 array pointer.
 * It returns when the record is finished (either OK or with error).
 * 
 * @param ir Pointer to array, where the timings should be stored
 * @return 0 on success, error code otherwise
 * 
 * TBD: implement error codes (example: no IR command recorded)
 */
 
//Timer 1 configuration: falling edge, normal mode, prescaler of 64 (4 us)
//ICNC1 ICES1 – WGM13 WGM12 CS12 (CS11) (CS10) -> falling edge detection
void timer1conf(){ TCCR1B |= 0b000000011; }

void startTimer1(){ TIMSK1 |= (1<<ICIE1); } 		//enables timer interrupt

void stopTimer1(){ TIMSK1 &= ~(1<<ICIE1); } 		//disables timer interrupt
 
//return 1 on timeout
//return 2, when exceeded MAX_IR_EDGES 
//return 0 on success
uint8_t ir_record_command(uint16_t * ir)
{
	DDRB &= ~(1<<PB0);								//configure input capture pin as input
    PORTB |= (1<<PB0);								//activate input capture pin internal pullup
    
    edge = 2;
    uint8_t loop = 1;
    
    timer0conf(1);
	startTimer0();									//start error check
	NRcheck = 1;									//checking for timeout
	timer1conf();									//init timer1
	startTimer1();									//start timer1 interrupt
	uart_sendstring("start:\n\r");
	recorded = 0;
	
	while(loop){ if((NRcheck == 2) || (NRcheck == 4) || (NRcheck == 5)){loop = 0;} }
	_delay_ms(10);
	
	if(NRcheck == 2){return 1;}
	if(NRcheck == 5){return 2; }
	if(NRcheck == 4){
		uart_sendstring("\n\rEnd of Signal detected");
		char array[6];
		for(uint8_t u = 1; u < recorded; u++){
			uart_sendstring("\r\n");
			int_to_str(ir_timings[u], array);
			ir[u] = ir_timings[u];
			uart_sendstring(array);
			_delay_ms(5);
		}
	}
	_delay_ms(100);
	return 0;	
}

ISR(TIMER1_CAPT_vect){
	TCNT1 = 0;
	NRcheck = 3;
	if(edge == 2){ edge = 1; TCCR1B |= (1<<ICES1); goto out;} //to ignore the value until the signal happens
	if(edge){
		NRcount = 0;
		edge = 0;
		TCCR1B &= ~(1<<ICES1); 						//change detection of input to falling edges
		ir_timings[recorded] = ICR1*4;
		goto out;
	}
	if(!edge){
		NRcount = 0;
		edge = 1;
		TCCR1B |= (1<<ICES1); 						//change detection of input to rising edges
		ir_timings[recorded] = ICR1*4;
		goto out;
	}
	out:
	recorded++;
	if(recorded >= MAX_IR_EDGES){ NRcheck = 5; stopTimer0(); stopTimer1();}
}
