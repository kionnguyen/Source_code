#include "MKL46Z4.h"
#include "i2c.h"
#include "delay.h"

void init_I2C0(void)
{
		//Enable clock for I2C0 and PORTE
		SIM->SCGC4|=SIM_SCGC4_I2C0_MASK;
		SIM->SCGC5|=SIM_SCGC5_PORTE_MASK;
	
		//Config SCL and SDA pin
		PORTE->PCR[I2C0_SCL]|= PORT_PCR_MUX(5);
		PORTE->PCR[I2C0_SDA]|= PORT_PCR_MUX(5);
	
		//Enable pullup resistor SCL
		PORTE->PCR[I2C0_SCL]|= PORT_PCR_PS_MASK |PORT_PCR_PE_MASK;
		//Enable pullup resistor SDA
		PORTE->PCR[I2C0_SDA]|= PORT_PCR_PS_MASK |PORT_PCR_PE_MASK;
	
		//Config I2C baud rate, SDA hold time, SCL stop hold time and SCL start hold time
		//Mul(02) = 4, ICR(0x00) => SCL divider = 20 
		/*
			I2C baud rate = bus speed (Hz)/(mul × SCL divider)
			SDA hold time = bus period (s) × mul × SDA hold value
			SCL start hold time = bus period (s) × mul × SCL start hold value
			SCL stop hold time = bus period (s) × mul × SCL stop hold value
		*/
		I2C0->F |= I2C_F_MULT(02) | I2C_F_ICR(0x00);
		//Enable I2C0
		I2C0->C1 |= I2C_C1_IICEN_MASK;
}


unsigned char I2C_SingleByteRead(unsigned char DEV_ADR, unsigned char REG_ADR)
{   
		unsigned char dummy_read = 0;
		unsigned char data = 0;
		//Select transmit mode
		I2C0->C1 |= I2C_C1_TX_MASK;
		//Start I2C
		I2C0->C1 |= I2C_C1_MST_MASK;
		
		//Send MAG device address with write bit
		I2C0->D = WRITE(DEV_ADR);
		//Wait for transmission: IICIF bit = 1 means there is an interrupt
		while((I2C0->S & I2C_S_IICIF_MASK) == 0){}
		//Clear IICIF flag
		I2C0->S |= I2C_S_IICIF_MASK;
		//Wait for ACK
		while ((I2C0->S & I2C_S_RXAK_MASK) != 0){}
			
		//Send MAG register address
		I2C0->D = REG_ADR;
		//Wait for transmission: IICIF bit = 1 means there is an interrupt
		while((I2C0->S & I2C_S_IICIF_MASK) == 0){}
		//Clear IICIF flag
		I2C0->S |= I2C_S_IICIF_MASK;
		//Wait for ACK
		while ((I2C0->S & I2C_S_RXAK_MASK) != 0){}
			
		//Repeated start to change to read mode
		I2C0->C1 |= I2C_C1_RSTA_MASK;
			
		//Send MAG device address and a Read Bit
		I2C0->D = READ(DEV_ADR);
		//Wait for transmission: IICIF bit = 1 means there is an interrupt
		while((I2C0->S & I2C_S_IICIF_MASK) == 0){}
		//Clear IICIF flag
		I2C0->S |= I2C_S_IICIF_MASK;
		//Wait for ACK
		while ((I2C0->S & I2C_S_RXAK_MASK) != 0){}
		
		//Sending NAK to ensure right after the data is read, NAK signal is sent
		I2C0->C1 |= I2C_C1_TXAK_MASK;
		//Set the I2C in Receiver Mode to read data from MAG3110
		I2C0->C1 &= (~I2C_C1_TX_MASK);
				
		//Read dummy data
		dummy_read = I2C0->D;
		//Wait for transmission: IICIF bit = 1 means there is an interrupt
		while((I2C0->S & I2C_S_IICIF_MASK) == 0){}
		//Clear IICIF flag
		I2C0->S |= I2C_S_IICIF_MASK;
		
		//Read real data
		data = I2C0->D;
		//Wait for transmission: IICIF bit = 1 means there is an interrupt
		while((I2C0->S & I2C_S_IICIF_MASK) == 0){}
		//Clear IICIF flag
		I2C0->S |= I2C_S_IICIF_MASK;
			
		//Stop I2C
		I2C0->C1 &= (~I2C_C1_MST_MASK);
		// Clear Transmit Nack by setting TXAK to 0
		I2C0->C1 &= ~(I2C_C1_TXAK_MASK);
			
		delay();
		return data;
}

void I2C_MultipleByteRead(unsigned char DEV_ADR, unsigned char REG_ADR, int num_bytes) {
    unsigned char dummy_data = 0;
    // Start I2C transmission in Master Transmit mode
    I2C0->C1 |= I2C_C1_TX_MASK | I2C_C1_MST_MASK;
    
    // Send device address with write bit
    I2C0->D = WRITE(DEV_ADR);
    while ((I2C0->S & I2C_S_IICIF_MASK) == 0) {} 
    I2C0->S |= I2C_S_IICIF_MASK; 
    while ((I2C0->S & I2C_S_RXAK_MASK) != 0){}
    
    // Send register address
    I2C0->D = REG_ADR;
    while ((I2C0->S & I2C_S_IICIF_MASK) == 0) {} 
    I2C0->S |= I2C_S_IICIF_MASK; 
    while ((I2C0->S & I2C_S_RXAK_MASK) != 0){}
    
    // Send repeated start signal
    I2C0->C1 |= I2C_C1_RSTA_MASK;
    
    // Send device address with read bit
    I2C0->D = READ(DEV_ADR);
    while ((I2C0->S & I2C_S_IICIF_MASK) == 0) {}
    I2C0->S |= I2C_S_IICIF_MASK; 
    while ((I2C0->S & I2C_S_RXAK_MASK) != 0){}
    
    // Switch to receiver mode
    I2C0->C1 &= ~I2C_C1_TX_MASK;
    
    // Read dummy data to initiate reading process
    dummy_data = I2C0->D;
    
    for (int i = 0; i < num_bytes; i++) {
        while ((I2C0->S & I2C_S_IICIF_MASK) == 0) {}
        I2C0->S |= I2C_S_IICIF_MASK; 
     
        if (i == num_bytes - 2) {
            // Set NACK before reading the second to last byte
            I2C0->C1 |= I2C_C1_TXAK_MASK;
        } else if (i == num_bytes - 1) {
            // After reading the last byte, generate a stop condition
            I2C0->C1 &= ~I2C_C1_MST_MASK;
        }
        
        // Read data
        DATA_READ[i] = I2C0->D;
    }
    
    // Clear NACK signal
    I2C0->C1 &= ~I2C_C1_TXAK_MASK;
    delay();
}

void I2C_SingleByteWrite(unsigned char DEV_ADR, unsigned char REG_ADR, unsigned char DATA)
{
		//Select transmit mode
		I2C0->C1 |= I2C_C1_TX_MASK;
		//Start I2C
		I2C0->C1 |= I2C_C1_MST_MASK;
		
		//Send MAG device address with write bit
		I2C0->D = WRITE(DEV_ADR);
		//Wait for transmission: IICIF bit = 1 means there is an interrupt
		while((I2C0->S & I2C_S_IICIF_MASK) == 0){}
		//Clear IICIF flag
		I2C0->S |= I2C_S_IICIF_MASK;
		//Wait for ACK
		while ((I2C0->S & I2C_S_RXAK_MASK) != 0){}
			
		//Send MAG register address
		I2C0->D = REG_ADR;
		//Wait for transmission: IICIF bit = 1 means there is an interrupt
		while((I2C0->S & I2C_S_IICIF_MASK) == 0){}
		//Clear IICIF flag
		I2C0->S |= I2C_S_IICIF_MASK;
		//Wait for ACK
		while ((I2C0->S & I2C_S_RXAK_MASK) != 0){}
			
		//Send data
		I2C0->D = DATA;
		//Wait for transmission: IICIF bit = 1 means there is an interrupt
		while((I2C0->S & I2C_S_IICIF_MASK) == 0){}
		//Clear IICIF flag
		I2C0->S |= I2C_S_IICIF_MASK;
		//Wait ACK reg address from MAG
		while ((I2C0->S & I2C_S_RXAK_MASK) != 0){}
		
		//Stop
		I2C0->C1&=(~I2C_C1_MST_MASK);
		
		delay();
}

void I2C_MultipleByteWrite(unsigned char DEV_ADR, unsigned char REG_ADR, int num_bytes, unsigned char data_bytes[])
{
		//Select transmit mode
		I2C0->C1 |= I2C_C1_TX_MASK;
		//Start I2C
		I2C0->C1 |= I2C_C1_MST_MASK;
		
		//Send MAG device address with write bit
		I2C0->D = WRITE(DEV_ADR);
		//Wait for transmission: IICIF bit = 1 means there is an interrupt
		while((I2C0->S & I2C_S_IICIF_MASK) == 0){}
		//Clear IICIF flag
		I2C0->S |= I2C_S_IICIF_MASK;
		//Wait for ACK
		while ((I2C0->S & I2C_S_RXAK_MASK) != 0){}
			
		//Send MAG register address
		I2C0->D = REG_ADR;
		//Wait for transmission: IICIF bit = 1 means there is an interrupt
		while((I2C0->S & I2C_S_IICIF_MASK) == 0){}
		//Clear IICIF flag
		I2C0->S |= I2C_S_IICIF_MASK;
		//Wait for ACK
		while ((I2C0->S & I2C_S_RXAK_MASK) != 0){}
			
		// Send the multiple bytes of data
		for(int i = 0;i < num_bytes; i++)
		{
			I2C0->D = data_bytes[i];
			//Wait for transmission: IICIF bit = 1 means there is an interrupt
			while((I2C0->S & I2C_S_IICIF_MASK) == 0){}
			//Clear IICIF flag
			I2C0->S |= I2C_S_IICIF_MASK;
			//Wait for ACK
			while ((I2C0->S & I2C_S_RXAK_MASK) != 0){}
		}
		//Stop 
		I2C0->C1&=(~I2C_C1_MST_MASK);
		delay();
}
