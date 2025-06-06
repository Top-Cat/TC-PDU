#include "Serial0.h"

#ifndef HardwareSerial_h
#define HardwareSerial_h

#include <inttypes.h>
#include "Stream.h"

// Register names for BITS are normalized to the unnumbered forms because they are not different where there are two USARTs.
#define RXEN	4
#define TXEN	3
#define RXCIE	7
#define UDRIE	5
#define U2X		1

const uint8_t _rxen		= (1 << RXEN);
const uint8_t _txen		= (1 << TXEN);
const uint8_t _rxcie	= (1 << RXCIE);
const uint8_t _udrie	= (1 << UDRIE);
const uint8_t _u2x		= (1 << U2X);

#define SERIAL_BUFFER_SIZE 16
/* WARNING
* BUFFER SIZES MUST BE POWERS OF TWO - The compiler misses some of the
* optimization possible with the % opperator. only a small portion of it, but as there's no compelling reason NOT
* to use a power of two size, and there are some extrenely flash-constrained parts that have a USART (I'm thinking of the 2313 in particular
* I couldn't justify not explicitly optimizing the % SERIAL_BUFFER_SIZE to a & (SERIAL_BUFFER_SIZE -1))
*/

struct ring_buffer;

struct ring_buffer
{
	unsigned char buffer[SERIAL_BUFFER_SIZE];
	uint8_t head;
	uint8_t tail;
};

inline void store_char(unsigned char c, ring_buffer *buffer) {
	uint8_t i = (buffer->head + 1) % SERIAL_BUFFER_SIZE;

	// if we should be storing the received character into the location
	// just before the tail (meaning that the head would advance to the
	// current location of the tail), we're about to overflow the buffer
	// and so we don't write the character or advance the head.
	if (i != buffer->tail) {
			buffer->buffer[buffer->head] = c;
			buffer->head = i;
	}
}

class HardwareSerial : public Stream {
private:
		volatile ring_buffer *_tx_buffer;
		volatile uint8_t *_ubrrh;
		volatile uint8_t *_ubrrl;
		volatile uint8_t *_ucsra;
		volatile uint8_t *_ucsrb;
		volatile uint8_t *_ucsrc;
		volatile uint8_t *_udr;
public:
		volatile ring_buffer *_rx_buffer;
		HardwareSerial(
			ring_buffer *rx_buffer,
			ring_buffer *tx_buffer,
			volatile uint8_t *ubrrh, volatile uint8_t *ubrrl,
			volatile uint8_t *ucsra, volatile uint8_t *ucsrb, volatile uint8_t *ucsrc,
			volatile uint8_t *udr
		);

		void begin(long);
		void end();
		// Basic printHex() forms for 8, 16, and 32-bit values
		void printHex(const uint8_t b);
		void printHex(const uint16_t w, bool s = 0);
		void printHex(const uint32_t l, bool s = 0);
		// printHex(signed) and printHexln() - trivial implementation;
		void printHex(const int8_t b) { printHex((uint8_t) b); }
		void printHex(const char b) { printHex((uint8_t) b); }
		// The pointer-versions for mass printing uint8_t and uint16_t arrays.
		uint8_t* printHex(uint8_t* p, uint8_t len, char sep = 0);
		uint16_t* printHex(uint16_t* p, uint8_t len, char sep = 0, bool s = 0);
		virtual int available(void);
		virtual int peek(void);
		virtual int read(void);
		virtual void flush(void);
		virtual size_t write(uint8_t);
		operator bool();
};

#endif

extern HardwareSerial Serial;
extern HardwareSerial Serial1;
