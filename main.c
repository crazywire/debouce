/*
 * debounce.c
 *
 * Created: 2018-01-21 6:33:07 AM
 * Authors: Ayan & Abdullahi 
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

const short min_sample_points =17;
const short total_sample_points = 20;
const float delta_t = 100e-6;
const float fclk = 14.7456e6;

unsigned short button_status, confirmed_button_status;
unsigned short sample_counts, good_sample_counts;

//Enumarate all possible states
//and assign them to the variable my_ccurent_state 
enum fsm_states{
	low, 
	trans_lh, 
	high, 
	trans_hl} my_current_state;



/*++++++EXTERNAL INTERRUPT SERVICE ROUNTINE+++++++
Actual undebounced signal is written to PB0.
My current state variable is used to change states
*/
ISR(INT0_vect){

	button_status = (PIND & (1<<PD2));
	
	//Write the actual signal to PB0
	if(button_status){
		PORTB |= (1<<PB0);
	}else{
		PORTB &= ~(1<<PB0);
	}
	
	//Change state from high/low to transition
	if(my_current_state == low){
		if(button_status){
			//current state is in transition from low to high
			my_current_state = trans_lh;
			//reset all counters
			good_sample_counts = 0;
			sample_counts=0;
			TCNT0 =0;
		}
	}
	
	if(my_current_state == high){
		if(!(button_status)){
			//current state is in transition from high to low
			my_current_state = trans_hl;
			//reset all counters
			good_sample_counts = 0;
			sample_counts=0;
			TCNT0 =0;
		}
	}
	
}

/*+++++OUTPUT COMPARE INTERRUPT SERVICE ROUNTINE+++++
Samples are incremented if the state is in transition
*/
ISR(TIMER0_COMPA_vect){
	
	//increment samples
	if(sample_counts < total_sample_points){
		sample_counts++; //count the samples
		if((my_current_state == trans_hl) || (my_current_state == trans_lh)){
			//count good samples while state is in transition
			good_sample_counts++; 
		}
	}else{
		//reset all counters
		good_sample_counts = 0;
		sample_counts=0;
		TCNT0 =0;
	}
}

/*++++++++++++DEBOUNCE ROUTINE++++++++++++++++
If enough samples are collected:
Button signal is debounced
The current state is changed back to high or low
*/
void debounce_button(){
	
	//Check if enough samples were collected
	if(good_sample_counts >= min_sample_points && sample_counts <=total_sample_points)
	{
		//check if still in transition
		if(my_current_state == trans_lh){
			//write debounced signal to this variable
			confirmed_button_status = 1;
			//Set state to high
			my_current_state = high;
			//clear all counters
			good_sample_counts = 0;
			sample_counts=0;
			TCNT0 =0;
		}
		
		if(my_current_state == trans_hl)
		{
			//write debounced signal to this variable
			confirmed_button_status = 0;
			//change state to low
			my_current_state = low;
			//clear counters
			good_sample_counts = 0;
			sample_counts=0;
			TCNT0 =0;
		}
	}
}

int main(void)
{
	DDRB = 0xFF;
	DDRD = 0;
	
	//Intialize both PORTB and my current state to low
	PORTB = 0;
	my_current_state =low;
	button_status = 0;
	confirmed_button_status = 0;
	}

	//Initialize all counters to zero
	sample_counts = 0;
	good_sample_counts = 0;
	
	//Enable external interrupt on PD2
	EIMSK |= (1<<INT0);
	EICRA |= (1<<ISC00);
	
	//Start timer0 and enable Output Compare interrupt 
	TIMSK0 |= (1<<OCIE0A);
	TCCR0B |= (1<<CS00);
	TCNT0 = 0;
	OCR0A = (fclk*delta_t/total_sample_points); //Set the trigger of the interrupt
	
	sei(); //Enable global interrupt
    
    while (1) 
    {
	
	//Call debounce function
	debounce_button();
	
	//Write debounced signal to PB1
	if(confirmed_button_status){
		PORTB |= (1<<PB1);
	}else{
		PORTB &= ~(1<<PB1);
	}
    }
}

