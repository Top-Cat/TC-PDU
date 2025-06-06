#if !defined(WIRE_S_H)
#define WIRE_S_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "Stream.h"

#define I2C_BUFFER_LENGTH 32

#define I2C_INTR_FLAG_INIT do{}while(0)
#define I2C_INTR_FLAG_ON   do{}while(0)
#define I2C_INTR_FLAG_OFF  do{}while(0)

struct i2cStruct {
	uint8_t  Buffer[I2C_BUFFER_LENGTH];      // Tx/Rx Buffer                      (ISR)
	volatile size_t   rxBufferIndex;         // Rx Index                          (User&ISR)
	volatile size_t   rxBufferLength;        // Rx Length                         (ISR)
	volatile size_t   txBufferIndex;         // Tx Index                          (User&ISR)
	volatile size_t   txBufferLength;        // Tx Length                         (User&ISR)
	volatile char     startCount;            // repeated START count              (User&ISR)
	volatile uint16_t Addr;                  // Tx/Rx address                     (User&ISR)
	bool (*user_onAddrReceive)(uint16_t, uint8_t);  // Slave Addr Callback Function      (User)
	void (*user_onReceive)(size_t);          // Slave Rx Callback Function        (User)
	void (*user_onRequest)(void);            // Slave Tx Callback Function        (User)
	void (*user_onStop)(void);               // Stop Callback Function            (User)
};

extern "C" void i2c_isr_handler(struct i2cStruct* i2c);

class i2c_tinyS : public Stream {
	private:
	static struct i2cStruct i2cData;
	friend void i2c_isr_handler(void);
	public:
	struct i2cStruct* i2c;
	i2c_tinyS();
	~i2c_tinyS();
	static void begin_(struct i2cStruct* i2c, uint8_t address, uint8_t mask);
	inline void begin(int address) {
		begin_(i2c, (uint8_t)address, 0);
	}
	inline void begin(uint8_t address, uint8_t mask) {
		begin_(i2c, address, mask);
	}
	size_t write(uint8_t data);
	inline size_t write(unsigned long n) { return write((uint8_t)n); }
	inline size_t write(long n)          { return write((uint8_t)n); }
	inline size_t write(unsigned int n)  { return write((uint8_t)n); }
	inline size_t write(int n)           { return write((uint8_t)n); }
	size_t write(const uint8_t* data, size_t quantity);
	inline size_t write(const char* str) { write((const uint8_t*)str, strlen(str)); return 0; }
	inline int available(void) { return i2c->rxBufferLength - i2c->rxBufferIndex; }
	static int read_(struct i2cStruct* i2c);
	inline int read(void) { return read_(i2c); }
	static int peek_(struct i2cStruct* i2c);
	inline int peek(void) { return peek_(i2c); }
	static uint8_t readByte_(struct i2cStruct* i2c);
	inline uint8_t readByte(void) { return readByte_(i2c); }
	static uint8_t peekByte_(struct i2cStruct* i2c);
	inline uint8_t peekByte(void) { return peekByte_(i2c); }
	inline void flush(void) {}
	inline uint16_t getRxAddr(void) { return i2c->Addr; }
	inline size_t getTransmitBytes(void) { return i2c->txBufferIndex; }
	inline void onAddrReceive(bool (*function)(uint16_t, uint8_t)) { i2c->user_onAddrReceive = function; }
	inline void onReceive(void (*function)(size_t)) { i2c->user_onReceive = function; }
	inline void onRequest(void (*function)(void)) { i2c->user_onRequest = function; }
	inline void onStop(void (*function)(void)) { i2c->user_onStop = function; }
};

extern i2c_tinyS TinyWireS;

#endif
