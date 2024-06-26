#include "MKL46Z4.h"
#include "slcd.h"
#include "mag.h"
#include "i2c.h"
#include "switch.h"
#include "led.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>



unsigned char DATA_READ[6];
short int MAG_DATA_READ_AXIS[3];
short int MAG_DATA_MAX_AXIS[3];
short int MAG_DATA_MIN_AXIS[3];
short int MAG_DATA_AVERAGE_AXIS[3];
short int MAG_DATA_HI_CALIBRATED[3];
short int ANGLE;

typedef enum {
	STOP,
	ACTIVE,
	ACQ,
	CAL,
	PAUSE
} CompassState;

CompassState state = STOP;

bool SW3push = false;
bool SW1push = false;

unsigned char    MAGprint[5] = "";

int32_t volatile msTicks = 0;

void init_SysTick()
{
	SysTick->LOAD = (SystemCoreClock / 1000) - 1; //configured the SysTick to count in 1ms 
	/* Select Core Clock & Enable SysTick & Enable Interrupt */
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

void Delay_Systick (uint32_t TICK) { 
	while (msTicks < TICK-1); 
	msTicks = 0; // Reset counter 
}

void SysTick_Handler (void) { // SysTick interrupt Handler
	msTicks++;
}


void Green_Led_Blink()
{
	PTD->PTOR |= (1 << GREEN);
	Delay_Systick(500);
}

void Red_Led_Blink()
{
	PTE->PTOR |= (1 << RED);
	Delay_Systick(250);
}

void PORTC_PORTD_IRQHandler(void)
{
	int sw1_state, sw2_state; //Indicates the status of SW1 and SW2 (if they are pressed or not)
	
	//Read Switch status
	sw1_state = (PORTC->PCR[SW1] & (1 << 24)) >> 24;
	sw2_state = (PORTC->PCR[SW2] & (1 << 24)) >> 24;
	
	//Update state variable of 2 switch and clear interrupt flag
	if (sw1_state == 1) {
		SW1push = 1; 
		PORTC->PCR[SW1] |= (1 << 24); 
	}
	if (sw2_state == 1) {
		SW3push = 1; 
		PORTC->PCR[SW2] |= (1 << 24); 
	}
}

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "pin_mux.h"
int main(void)
{
    SLCD_Init();
	init_Led();
	init_SW();
	init_SysTick();
	init_I2C0();
	NVIC_EnableIRQ(PORTC_PORTD_IRQn);

    while(1){
        if(SW1push == true){
          // Clear the flag
            SW1push = false;
            if(state == STOP){
                state = ACQ;
            }else if(state == ACQ){
                state = CAL;
            }else if(state == CAL){
                state = ACTIVE;
            }else if(state == ACTIVE) {
	        state = PAUSE;
	    }else {
	        state = ACTIVE;
	    }
        }
	else if(SW3push == true) {
          // Clear the flag
            SW3push = false;
            state = STOP;
        }
				
    if(state == STOP){
		PTE->PCOR |= (1 << RED);
		PTD->PCOR |= (1 << GREEN);
		for(int i = 0; i < 3; i++)
		{
			MAG_DATA_MAX_AXIS[i]=0;
			MAG_DATA_MIN_AXIS[i]=0;
			MAG_DATA_READ_AXIS[i]=0;
		}
		SLCD_WriteMsg((unsigned char *)"STOP");
    }
	else if(state == ACTIVE) {
		PTE->PSOR |= (1 << RED);
		MAG3110_Run();
		Green_Led_Blink();
		snprintf(MAGprint,5,"%4d",ANGLE);
		SLCD_WriteMsg(MAGprint);
	}
	else if(state == ACQ) {
		MAG3110_Init();
		MAG3110_Acq();
		SLCD_WriteMsg((unsigned char *)"ACQ");
	}
	else if(state == CAL) {
		MAG3110_Cal();
		SLCD_WriteMsg((unsigned char *)"CAL1");
        }
	else {
		PTD->PSOR |= (1 << GREEN);
		Red_Led_Blink();
		SLCD_WriteMsg((unsigned char *)"REST");
	}
    }
}
