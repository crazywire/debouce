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
	
	
	//Apply original signal to PB0
	if (button_status)
	{
		PORTB |= (1<<PB0); 
	}else{
		PORTB &= ~(1<<PB0);
	}
	
	//If the switch is opened, transition from high to low
	if (my_current_state == high)
	{
		if (!(button_status))
		{
			my_current_state = trans_hl;
			sample_counts = 0;
			good_sample_counts = 0;
	
		}
	}
	
	//If the switch is closed, transition from low to high 
	if (my_current_state == low)
	{
		if (button_status)
		{
			my_current_state = trans_lh;
			sample_counts = 0;
			good_sample_counts = 0;
		}
	}
}

ISR(TIMER0_COMPA_vect){
	
	if (sample_counts<= total_sample_points)
	{
		sample_counts++;
		}else{
			sample_counts = 0;
			good_sample_counts = 0;
	}
	
	//if in transition count the number of the same successive lows or highs
	if (my_current_state == trans_lh)
	{
		if (button_status)
		{
			good_sample_counts++;
		}
	}
	
	if (my_current_state == trans_hl)
	{
		if (!(button_status))
		{
			good_sample_counts++;
		}
	}

}


void debounce_button(){
	
	if (my_current_state == trans_lh)
	{
		if (good_sample_counts >= min_sample_points)
		{
			confirmed_button_status = 1;
			my_current_state = high;
		}
	}
	if (my_current_state == trans_hl)
	{
		if (good_sample_counts >= min_sample_points)
		{
			confirmed_button_status = 0;
			my_current_state = low;
		}
	}
}

int main(void)
{
	DDRB |= (1<<PB0) | (1<<PB1);
	DDRD &= ~(1<<PD2);
	
	
	//Decide if the switch is open or closed

	if (PIND & (1<<PD2))
	{
		my_current_state = high;
		PORTB |= (1<<PB0) | (1<<PB1);
	}else{
		my_current_state = low;
		PORTB &= ~ (1<<PB0) & ~(1<<PB1);
	}
	
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

