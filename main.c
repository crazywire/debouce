/*
 * debounce.c
 *
 * Created: 2018-01-21 6:33:07 AM
 * Author : Abdul
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

const short min_sample_points =17;
const short total_sample_points = 20;
const float delta_t = 100e-6;
const float fclk = 14.7456e6;

unsigned short button_status, confirmed_button_status;
unsigned short sample_counts, good_sample_counts;

enum fsm_states{
	low, 
	trans_lh, 
	high, 
	trans_hl} my_current_state;

ISR(INT0_vect){

	button_status = (PIND & (1<<PD2));
	
	
	
}

ISR(TIMER0_COMPA_vect){
	
	
}


void debounce_button(){
	
	
}

int main(void)
{
	DDRB |= (1<<PB0) | (1<<PB1);
	DDRD &= ~(1<<PD2);
	
	
	
	EIMSK = (1<<INT0);
	EICRA |= (1<<ISC00);
	
	sample_counts = 0;
	button_status = 0;
	confirmed_button_status = 0;
	
	TIMSK0 |= (1<<OCIE0A);
	TCCR0B |= (1<<CS00);
	TCNT0 = 0;
	OCR0A = 73;
	
	sei();
    
    while (1) 
    {
		debounce_button();
		
		if(confirmed_button_status){
			PORTB |= (1<<PB1);
		}else{
			PORTB &= ~(1<<PB1);
		}
    }
}

