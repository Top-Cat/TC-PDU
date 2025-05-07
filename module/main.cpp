#include "Serial0.h"
#include "HLW8032.h"

#include "WireS.h"
#include <avr/io.h>
#include <avr/eeprom.h>

#define RELAY_PIN_OFF DDA1
#define RELAY_PIN DDA0
#define RELAY_PINS (_BV(RELAY_PIN_OFF) | _BV(RELAY_PIN))
// Need to remap timer if these are changed
#define RED_PIN DDA3
#define GREEN_PIN DDA2

uint8_t EEMEM i2cAddr = 0x0a;
uint8_t lastRelayCommand = 2;
HLW8032 HL;

ISR(TIMER2_OVF_vect) {
	PORTA &= ~RELAY_PINS;
	PORTB &= ~RELAY_PINS;
	TCCR2B &= ~(_BV(CS22)); // Stop timer 2
}

void setPWM(uint8_t RxByte) {
	// Bits 3 and 4 enable PWM
	uint8_t RED_PULSE = bitRead(RxByte, 3);
	uint8_t GREEN_PULSE = bitRead(RxByte, 4);
	TOCPMCOE = (GREEN_PULSE<<TOCC1OE) | (RED_PULSE<<TOCC2OE);

	// Byte 2 sets PWM period
	ICR1 = (TinyWireS.read() << 8) | 0xFF;
	// Byte 3 sets duty cycle
	OCR1B = OCR1A = (TinyWireS.read() << 8) | 0xFF;
}

void setPin(uint8_t pin, bool state) {
	bitWrite(PORTA, pin, state);
}

void setPinB(uint8_t pin, bool state) {
	bitWrite(PORTB, pin, state);
}

void I2C_RxHandler(unsigned int numBytes)
{
	if (numBytes != 3) return;

	uint8_t RxByte = TinyWireS.read();
	bool relayOn = bitRead(RxByte, 0);

	if (relayOn != lastRelayCommand) {
		lastRelayCommand = relayOn;

		// Start timer 2
		TCNT2 = 0;
		TCCR2B |= _BV(CS22);

		setPin(RELAY_PIN, relayOn);
		setPin(RELAY_PIN_OFF, !relayOn);
		setPinB(RELAY_PIN, relayOn);
		setPinB(RELAY_PIN_OFF, !relayOn);
	}

	setPin(RED_PIN, bitRead(RxByte, 1));
	setPin(GREEN_PIN, bitRead(RxByte, 2));

	// bit 5 = scale. 1 = 1024, 0 = 256
	bitWrite(TCCR1B, CS10, bitRead(RxByte, 5));

	setPWM(RxByte);
}

void I2C_TxHandler(void)
{
	uint8_t *raw = HL.GetRaw();

	for (uint8_t a = 0; a < HLW8032_DATA_LEN; a++)
	{
		TinyWireS.write(raw[a]);
	}
	TinyWireS.write(HL.PFData >> 24 & 0xFF);
	TinyWireS.write(HL.PFData >> 16 & 0xFF);
	TinyWireS.write(HL.PFData >> 8 & 0xFF);
	TinyWireS.write(HL.PFData & 0xFF);
	TinyWireS.write(0x55); // Stop byte
}

int main(void)
{
	REMAP = 1; // Remap TX0 to alternate pin, we're using the normal one for the relay
	ADCSRA = 0; // Disable ADC
	PRR = _BV(PRUSART1) | _BV(PRTIM0) | _BV(PRADC); // Turn off features we're not using
	sei(); // Enable interrupts or serial won't work
	HL.begin(Serial);

	DDRA = (1<<RED_PIN) | (1<<GREEN_PIN) | RELAY_PINS; // Set pins to output
	PORTA = 0; // Outputs low
	DDRB = RELAY_PINS;
	PORTB = 0;
	TOCPMSA0 = (1<<TOCC1S0) | (1<<TOCC2S0); // Enable mapping TIMER1 to LEDS
	// TOCPMCOE = (1<<TOCC1OE) | (1<<TOCC2OE); // Commented so that by default the LEDS are not tied to TIMER1

	// LED Timer
	TIMSK1 = 0;
	TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(COM1B0) | _BV(WGM11); // Clear on match, High at BOTTOM
	// 1110 = Fast PWM using ICRn
	// 0100 = CTC
	TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS12); // 256 prescaled, CTC via OCR1A
	ICR1 = 0x03FF;
	OCR1A = 0x01FF;
	OCR1B = 0x01FF;

	// Relay reset timer
	TIMSK2 = 1; // Enable interrupt
	TCCR2A = _BV(WGM21);
	TCCR2B = _BV(WGM23) | _BV(WGM22) | _BV(CS22);
	ICR2 = 0x03FF; // Tunable to change how long relay is energised

	// i2c init
	uint8_t addr = eeprom_read_byte(&i2cAddr);
	TinyWireS.begin(addr);
	TinyWireS.onReceive(I2C_RxHandler);
	TinyWireS.onRequest(I2C_TxHandler);

	while(1)
	{
		HL.SerialReadLoop();
	}
}
