#include "MKL46Z4.h"
#include "switch.h"

void init_SW(void)
{
		//Enable clock for port C
		SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;
		
	  //Config for SW1 and SW2: Set GPIO mode, enable pullup resistor and detect interrupt on falling edge
		PORTC->PCR[SW1] |= PORT_PCR_MUX(1) | (1 << 0) | (1 << 1) | (1 << 17) | (1 << 19);
		PORTC->PCR[SW2] |= PORT_PCR_MUX(1) | (1 << 0) | (1 << 1) | (1 << 17) | (1 << 19);
	  
		//Set pin to input
		PTC->PDDR &= (~(1 << SW1))&(~(1 << SW2));
}
