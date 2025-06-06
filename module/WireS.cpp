#include "WireS.h"

#define TWI_HIGH_NOISE_MODE 0

#ifndef TWAE
#define TWAE 0
#endif

#define SET_TENBIT   do { i2c->Addr |= 0x8000; } while(0)
#define CLEAR_TENBIT do { i2c->Addr &= 0x7FFF; } while(0)
#define IS_TENBIT (i2c->Addr & 0x8000)

struct i2cStruct i2c_tinyS::i2cData;

i2c_tinyS::i2c_tinyS() {
	i2c = &i2cData;
}

i2c_tinyS::~i2c_tinyS() {}

void i2c_tinyS::begin_(struct i2cStruct* i2c, uint8_t address, uint8_t mask) {
	I2C_INTR_FLAG_INIT;
	TWSA = (address << 1);
	TWSAM = mask;
	i2c->startCount = -1;
	TWSCRA = (_BV(TWSHE) | _BV(TWDIE) | _BV(TWASIE) | _BV(TWEN) | _BV(TWSIE));
}

size_t i2c_tinyS::write(uint8_t data) {
	if (i2c->txBufferLength < I2C_BUFFER_LENGTH) {
		i2c->Buffer[i2c->txBufferLength++] = data;
		return 1;
	}
	return 0;
}

size_t i2c_tinyS::write(const uint8_t* data, size_t quantity) {
	if (i2c->txBufferLength < I2C_BUFFER_LENGTH) {
		size_t avail = I2C_BUFFER_LENGTH - i2c->txBufferLength;
		uint8_t* dest = i2c->Buffer + i2c->txBufferLength;
		if (quantity > avail) quantity = avail;
		for (size_t count = quantity; count; count--) *dest++ = *data++;
		i2c->txBufferLength += quantity;
	}
	return 0;
}

int i2c_tinyS::read_(struct i2cStruct* i2c) {
	if (i2c->rxBufferIndex >= i2c->rxBufferLength) return -1;
	return i2c->Buffer[i2c->rxBufferIndex++];
}

int i2c_tinyS::peek_(struct i2cStruct* i2c) {
	if (i2c->rxBufferIndex >= i2c->rxBufferLength) return -1;
	return i2c->Buffer[i2c->rxBufferIndex];
}

uint8_t i2c_tinyS::readByte_(struct i2cStruct* i2c) {
	if (i2c->rxBufferIndex >= i2c->rxBufferLength) return 0;
	return i2c->Buffer[i2c->rxBufferIndex++];
}

uint8_t i2c_tinyS::peekByte_(struct i2cStruct* i2c) {
	if (i2c->rxBufferIndex >= i2c->rxBufferLength) return 0;
	return i2c->Buffer[i2c->rxBufferIndex];
}

void i2c_isr_handler() {
	struct i2cStruct *i2c = &(i2c_tinyS::i2cData);
	uint8_t status = TWSSRA;
	if ((status & (_BV(TWC) | _BV(TWBE)))) {
		i2c->startCount = -1;
		CLEAR_TENBIT;
		TWSSRA |= (_BV(TWASIF) | _BV(TWDIF) | _BV(TWBE));
		return;
	}
	if ((status & _BV(TWASIF)) || IS_TENBIT) {
		if ((status & _BV(TWAS))) {
			if (IS_TENBIT) i2c->Addr = (((i2c->Addr & 0b110) << 7) | TWSD);
			else {
				i2c->Addr = TWSD;
				i2c->startCount++;
				if ((i2c->Addr & 0b11111001) == 0b11110000) {
					SET_TENBIT;
					TWSCRB = (0b0011 | TWI_HIGH_NOISE_MODE);
					return;
				}
			}
			if (i2c->user_onAddrReceive != (void *)NULL) {
				i2c->rxBufferIndex = 0;
				if (!i2c->user_onAddrReceive(i2c->Addr, i2c->startCount)) {
					TWSCRB = (0b0111 | TWI_HIGH_NOISE_MODE);
					return;
				}
			}
			if ((status & _BV(TWDIR))) {
				i2c->txBufferLength = 0;
				if (i2c->user_onRequest != (void *)NULL) i2c->user_onRequest();
				i2c->txBufferIndex = 0;
			} else i2c->rxBufferLength = 0;
			} else {
			if ((status & _BV(TWDIR))) {
				if (i2c->user_onStop != (void *)NULL) i2c->user_onStop();
				} else {
				if (i2c->user_onReceive != (void *)NULL) {
					i2c->rxBufferIndex = 0;
					i2c->user_onReceive(i2c->rxBufferLength);
				}
			}
			i2c->startCount = -1;
			CLEAR_TENBIT;
			TWSSRA = _BV(TWASIF);
			return;
		}
		} else if ((status & _BV(TWDIF))) {
		if ((status & _BV(TWDIR))) {
			if (i2c->txBufferIndex < i2c->txBufferLength) TWSD = i2c->Buffer[i2c->txBufferIndex++];
			else {
				TWSCRB = (0b0010 | TWI_HIGH_NOISE_MODE);
				return;
			}
			} else {
			if (i2c->rxBufferLength < I2C_BUFFER_LENGTH) i2c->Buffer[i2c->rxBufferLength++] = TWSD;
			else {
				TWSCRB = (0b0110 | TWI_HIGH_NOISE_MODE);
				return;
			}
		}
	}
	TWSCRB = (0b0011 | TWI_HIGH_NOISE_MODE);
}

ISR(TWI_SLAVE_vect) {
	I2C_INTR_FLAG_ON;
	i2c_isr_handler();
	I2C_INTR_FLAG_OFF;
}

i2c_tinyS TinyWireS  = i2c_tinyS();
