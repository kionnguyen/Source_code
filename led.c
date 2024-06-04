#include "MKL46Z4.h"
#include "led.h"

void init_Led(void)
{
	//enable clock for led port
	SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK;
	//Config Green Led
	PORTD->PCR[GREEN] = PORT_PCR_MUX(1) ;
	PTD->PDDR |= (1 << GREEN);
	PTD->PSOR |= (1 << GREEN);

	//Config Red Led
	PORTE->PCR[RED] = PORT_PCR_MUX(1) ;
	PTE->PDDR |= (1 << RED);
	PTE->PSOR |= (1 << RED) ;
}
