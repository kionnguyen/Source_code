#ifndef __I2C_H__
#define __I2C_H__

#define I2C0_SCL                           (24)
#define I2C0_SDA                           (25)
#define READ(x)                            ((x<<1)|(0x01))
#define WRITE(x)                           ((x<<1)&(0xFE))

extern unsigned char DATA_READ[6];

void init_I2C0(void);
unsigned char I2C_SingleByteRead(unsigned char DEV_ADR, unsigned char REG_ADR);
void I2C_MultipleByteRead(unsigned char DEV_ADR,unsigned char REG_ADR, int num_bytes);
void I2C_SingleByteWrite(unsigned char DEV_ADR, unsigned char REG_ADR, unsigned char DATA);
void I2C_MultipleByteWrite(unsigned char DEV_ADR, unsigned char REG_ADR, int num_bytes, unsigned char data_bytes[]);

#endif
