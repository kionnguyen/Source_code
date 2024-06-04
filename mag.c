#include "MKL46Z4.h"
#include "i2c.h"
#include "mag.h"
#include "slcd.h"
#include <math.h>

//Init MAG3110
void MAG3110_Init(void)
{	
		unsigned char MAG_DEVICE_ID;

		MAG_DEVICE_ID = I2C_SingleByteRead(MAG_DEVICE_ADDRESS, MAG_DEVICE_ID_REGISTER_ADDRESS);
		
		//check device id
		while(MAG_DEVICE_ID != 0xC4) //WHO_AM_I default value of MAG3110
		{
			SLCD_WriteMsg((unsigned char *)"Err");
		}
		
		//Set continuous measurements with ODR = 80Hz, OSR = 1
		I2C_SingleByteWrite(MAG_DEVICE_ADDRESS,MAG_CTRL_REG1, 0x01);
		I2C_SingleByteWrite(MAG_DEVICE_ADDRESS,MAG_CTRL_REG2, 0x80);
}

//Acquire Max and Min value in each axis
void MAG3110_Acq() 
{
    int index;

    // Reading data from MAG3110
    I2C_MultipleByteRead(MAG_DEVICE_ADDRESS, MAG_OUT_X_MSB, 6);

    // Combining MSB and LSB data 
    for (index = 0; index < 3; index++) 
    {
        MAG_DATA_READ_AXIS[index] = ((short int)((DATA_READ[2 * index] << 8) | DATA_READ[2 * index + 1]));
    }

    // Find the Max and Min values
    for (index = 0; index < 3; index++) 
    {
        if (MAG_DATA_MAX_AXIS[index] == 0 && MAG_DATA_MIN_AXIS[index] == 0) {
            MAG_DATA_MAX_AXIS[index] = MAG_DATA_READ_AXIS[index];
            MAG_DATA_MIN_AXIS[index] = MAG_DATA_READ_AXIS[index];
        } 
        else {
            if (MAG_DATA_READ_AXIS[index] > MAG_DATA_MAX_AXIS[index]) {
                MAG_DATA_MAX_AXIS[index] = MAG_DATA_READ_AXIS[index];
            }
            if (MAG_DATA_READ_AXIS[index] < MAG_DATA_MIN_AXIS[index]) {
                MAG_DATA_MIN_AXIS[index] = MAG_DATA_READ_AXIS[index];
            }
        }
    }
}

//Calibrate MAG3110 for more accuracy based on max and min data
void MAG3110_Cal() 
{
    for (int index = 0; index < 3; index++) {
        MAG_DATA_AVERAGE_AXIS[index] = (MAG_DATA_MAX_AXIS[index] + MAG_DATA_MIN_AXIS[index]) / 2;
    }
}

//Calculate acquired data to Angle of compass with North pole is root
void MAG3110_Run() 
{
    int index;

    // Reading data from magnetometer along X, Y, Z axes
    I2C_MultipleByteRead(MAG_DEVICE_ADDRESS, MAG_OUT_X_MSB, 6);

    // Combining upper and lower bytes of data for each axis to get complete data along each axis
    for (index = 0; index < 3; index++) {
        MAG_DATA_READ_AXIS[index] = ((short int)((DATA_READ[2 * index] << 8) | DATA_READ[2 * index + 1]));
    }

    // Calculating the calibrated magnetometer data by subtracting the data offset from the raw uncalibrated data
    for (index = 0; index < 3; index++) {
        MAG_DATA_HI_CALIBRATED[index] = MAG_DATA_READ_AXIS[index] - MAG_DATA_AVERAGE_AXIS[index];
    }

    if (MAG_DATA_HI_CALIBRATED[1] == 0) {
        if (MAG_DATA_HI_CALIBRATED[0] > 0) {
            ANGLE = 0;
        } 
        else {
            ANGLE = 180;
        }
    } 
    else if (MAG_DATA_HI_CALIBRATED[1] < 0) {
        ANGLE = 270 - (atan((double)MAG_DATA_HI_CALIBRATED[0] / (double)MAG_DATA_HI_CALIBRATED[1]) * 57.29);
    } 
    else {
        ANGLE = 90 - (atan((double)MAG_DATA_HI_CALIBRATED[0] / (double)MAG_DATA_HI_CALIBRATED[1]) * 57.29);
    }
}
