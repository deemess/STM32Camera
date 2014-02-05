#include "i2csoft.h"

int i2c_host_address = 0;

void Start() {
	SET_SDA;	I2CDELAY;
	SET_SCL;	I2CDELAY;

	RESET_SDA;	I2CDELAY;
	RESET_SCL;	I2CDELAY;
}

void Stop() {
	RESET_SDA;	I2CDELAY;
	SET_SCL;	I2CDELAY;
	SET_SDA;	I2CDELAY;
}

void ReStart() {
	SET_SDA;	I2CDELAY;
	SET_SCL;	I2CDELAY;

	RESET_SDA;	I2CDELAY;
	RESET_SCL;	I2CDELAY;
}

int SendAddress(uint8_t address, uint8_t write) {
	uint8_t res = 0;
	uint8_t i=0;

	//send byte
	for(i=0; i<7; i++) {
		if(address & 0b10000000) {
			//1
			SET_SDA; I2CDELAY;
			SET_SCL; I2CDELAY;
			RESET_SCL; I2CDELAY;
			SET_SDA; I2CDELAY;
		} else {
			//0
			RESET_SDA; I2CDELAY;
			SET_SCL; I2CDELAY;
			RESET_SCL; I2CDELAY;
			SET_SDA; I2CDELAY;
		}
		address = address << 1;
	}

	if(write) {
		//0
		RESET_SDA; I2CDELAY;
		SET_SCL; I2CDELAY;
		RESET_SCL; I2CDELAY;
		SET_SDA; I2CDELAY;
	} else {
		//1
		SET_SDA; I2CDELAY;
		SET_SCL; I2CDELAY;
		RESET_SCL; I2CDELAY;
		SET_SDA; I2CDELAY;
	}

	//read ack
	I2CDELAY;
	SET_SCL;
	I2CDELAY;

	//check ack
	res = GPIO_ReadInputDataBit(I2CPORT, SDA);

	RESET_SCL;	I2CDELAY;
	RESET_SDA;	I2CDELAY;

	return res;
}


uint8_t SendByte(uint8_t byte) {
	uint8_t res = 0;
	uint8_t i=0;

	//send byte
	for(i=0; i<8; i++) {
		if(byte & 0b10000000) {
			//1
			SET_SDA; I2CDELAY;
			SET_SCL; I2CDELAY;
			RESET_SCL; I2CDELAY;
			SET_SDA; I2CDELAY;
		} else {
			//0
			RESET_SDA; I2CDELAY;
			SET_SCL; I2CDELAY;
			RESET_SCL; I2CDELAY;
			SET_SDA; I2CDELAY;
		}
		byte = byte << 1;
	}

	//read ack
	I2CDELAY;
	SET_SCL; I2CDELAY;
	//check ack

	res = GPIO_ReadInputDataBit(I2CPORT, SDA);

	RESET_SCL; I2CDELAY;
	RESET_SDA; I2CDELAY;

	return res;
}

uint8_t ReadByte(uint8_t ack) {
	uint8_t res = 0;
	uint8_t i=0;

	SET_SDA;

	//read byte
	for(i=0; i<8; i++) {
		res = res << 1;

		I2CDELAY;
		SET_SCL;
		I2CDELAY;

		if(GPIO_ReadInputDataBit(I2CPORT, SDA)) {
			res = res | 0b00000001;
		}

		RESET_SCL;
		I2CDELAY;
		I2CDELAY;

	}

	//ack
	if(ack) {
		RESET_SDA; I2CDELAY;
	} else {
		SET_SDA; I2CDELAY;
	}

	SET_SCL; I2CDELAY;
	RESET_SCL; I2CDELAY;
	RESET_SDA; I2CDELAY;

	return res;
}

Status I2C_ReadAddr(uint8_t* buf, uint32_t nbuf, uint8_t slaveAddress, uint8_t writeAddress)
{
	uint8_t res=0;
	uint8_t i=0;

	//start
	Start();

	//send address
	res = SendAddress(slaveAddress, 1);
	res |= SendByte(writeAddress);

	ReStart();

	//send address
	res |= SendAddress(slaveAddress, 0);

	//read data
	for(i=0; i<nbuf; i++) {
		if(i == nbuf-1) {
			//last byte without ack
			buf[i] = ReadByte(0);
		} else {
			buf[i] = ReadByte(1);
		}
	}

	//stop
	Stop();

	if(res)
		return Error;

	return Success;
}

Status I2C_WriteAddr(const uint8_t* buf, uint32_t nbuf,  uint8_t slaveAddress, uint8_t writeAddress)
{
	uint8_t res=0;
	uint8_t i=0;

	//start
	Start();

	//send address
	res = SendAddress(slaveAddress, 1);
	res |= SendByte(writeAddress);

	//send data
	for(i=0; i<nbuf; i++) {
		res |= SendByte(buf[i]);
	}

	//stop
	Stop();

	if(res)
		return Error;

	return Success;
}

void I2C_LowLevel_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	SET_SDA;
	SET_SCL;

	I2CPORTCLOCKENABLE;

	/* Configure I2C1 GPIOs *****************************************************/
	GPIO_InitStructure.GPIO_Pin = SCL | SDA;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(I2CPORT, &GPIO_InitStructure);

	SET_SDA;
	SET_SCL;
}


