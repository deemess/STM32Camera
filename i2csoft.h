#ifndef I2CSOFT_H
#define I2CSOFT_H

#include <stm32f10x.h>
#include "delay.h"


#define SCL GPIO_Pin_10
#define SDA GPIO_Pin_11

#define I2CPORT GPIOB
#define I2CPORTCLOCKENABLE RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE)

#define SET_SCL		GPIO_SetBits(I2CPORT, SCL)
#define RESET_SCL	GPIO_ResetBits(I2CPORT, SCL)
#define SET_SDA		GPIO_SetBits(I2CPORT, SDA)
#define RESET_SDA	GPIO_ResetBits(I2CPORT, SDA)

#define I2CDELAY	_delay_us(12)

#define OAR1_ADD0_Set           ((uint8_t)0x01)
#define OAR1_ADD0_Reset         ((uint8_t)0xFE)


typedef enum {Error = 0, Success = !Error } Status;

Status I2C_ReadAddr(uint8_t* buf, uint32_t nbuf, uint8_t slaveAddress, uint8_t writeAddress);
Status I2C_WriteAddr(const uint8_t* buf, uint32_t nbuf,  uint8_t slaveAddress, uint8_t writeAddress);
void I2C_LowLevel_Init();





#endif
