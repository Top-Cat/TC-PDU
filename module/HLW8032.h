#ifndef HLW8032_h
#define HLW8032_h

#include <stdbool.h>
#include <inttypes.h>
#include "HardwareSerial.h"

#define HLW8032_DATA_LEN 24

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

class HLW8032
{
	public:
		HLW8032();
		void begin(HardwareSerial& SerialData);
		void SerialReadLoop();
		uint8_t *GetRaw();

		uint32_t PFData;
	private:
		HardwareSerial *SerialID;
		uint8_t Index;
		uint8_t Data;
		uint8_t Check;

		bool BufferA = true;
		uint8_t SerialTempsA[HLW8032_DATA_LEN];
		uint8_t SerialTempsB[HLW8032_DATA_LEN];
		uint8_t *GetBuffer();
};


#endif
