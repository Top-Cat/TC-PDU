#include "HLW8032.h"

HLW8032::HLW8032()
{
}

void HLW8032::begin(HardwareSerial& SerialData)
{
	 SerialID = &SerialData;
	 SerialID->begin(4800);
}

uint8_t *HLW8032::GetBuffer() {
	return BufferA ? SerialTempsA : SerialTempsB;
}

void HLW8032::SerialReadLoop()
{
	while(SerialID->available() > 0) {
		Data = SerialID->read();
		if (Index >= 2 && Index < HLW8032_DATA_LEN) {
			uint8_t* buff = GetBuffer();
			buff[Index++] = Data;

			if (Index == HLW8032_DATA_LEN && Check == Data) {
				BufferA = !BufferA;
				
				if (bitRead(buff[20], 7) == 1) {
					PFData++;
				}
			} else {
				Check += Data;
			}
		} else if (Data == 0x5A && Index == 1) {
			GetBuffer()[Index++] = Data;
			Check = 0;
		} else if (Data >= 0xF0 || Data == 0x55 || Data == 0xAA) { // >= 0XFF = Overflow due to very low voltage, 0xAA = ERROR, 0x55 = NORMAL
			GetBuffer()[0] = Data;
			Index = 1;
		} else {
			Index = 0;
		}
	}
}

uint8_t *HLW8032::GetRaw() {
	return BufferA ? SerialTempsB : SerialTempsA;
}
