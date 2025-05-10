#define UBRR0H UBRR0H
#define UBRR1H UBRR1H
#define F_CPU 8000000

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "HardwareSerial.h"
	
//from here on out we rely on the normalized register names!


// Constructors ////////////////////////////////////////////////////////////////

HardwareSerial::HardwareSerial(ring_buffer *rx_buffer, ring_buffer *tx_buffer
	,volatile uint8_t *ubrrh, volatile uint8_t *ubrrl,
	volatile uint8_t *ucsra, volatile uint8_t *ucsrb, volatile uint8_t *ucsrc,
	volatile uint8_t *udr) {
	_rx_buffer = rx_buffer;
	_tx_buffer = tx_buffer;
	_ubrrh = ubrrh;
	_ubrrl = ubrrl;
	_ucsra = ucsra;
	_ucsrb = ucsrb;
	_ucsrc = ucsrc;
	_udr = udr;
}


// Public Methods //////////////////////////////////////////////////////////////

void HardwareSerial::begin(long baud) {
	uint16_t baud_setting;
	bool use_u2x = true;

try_again:

	if (use_u2x) {
		*_ucsra = _u2x;
		baud_setting = (F_CPU / 4 / baud - 1) / 2;
	} else {
		baud_setting = (F_CPU / 8 / baud - 1) / 2;
	}

	if ((baud_setting > 4095) && use_u2x) {
		use_u2x = false;
		goto try_again;
	}
	// assign the baud_setting, a.k.a. ubbr (USART Baud Rate Register)
	*_ubrrh = baud_setting >> 8;
	*_ubrrl = baud_setting;
	*_ucsrb = (_rxen | _txen | _rxcie);
	*_ucsrc = 0x26;
}

void HardwareSerial::end() {
	while (_tx_buffer->head != _tx_buffer->tail) {
		; // wait for buffer to end.
	}

		*_ucsrb = 0; //

	_rx_buffer->head = _rx_buffer->tail;
}

int HardwareSerial::available(void) {
	return (unsigned int)(SERIAL_BUFFER_SIZE + _rx_buffer->head - _rx_buffer->tail) & (SERIAL_BUFFER_SIZE - 1);
}

int HardwareSerial::peek(void) {
	if (_rx_buffer->head == _rx_buffer->tail) {
		return -1;
	} else {
		return _rx_buffer->buffer[_rx_buffer->tail];
	}
}

int HardwareSerial::read(void) {
	// if the head isn't ahead of the tail, we don't have any characters
	if (_rx_buffer->head == _rx_buffer->tail) {
		return -1;
	} else {
		unsigned char c = _rx_buffer->buffer[_rx_buffer->tail];
		_rx_buffer->tail = (_rx_buffer->tail + 1)	& (SERIAL_BUFFER_SIZE - 1);
		return c;
	}
}

void HardwareSerial::flush() {
	while (_tx_buffer->head != _tx_buffer->tail);
}

size_t HardwareSerial::write(uint8_t c) {
	/* Removed for space */
	return 1;
}

HardwareSerial::operator bool() {
	return true;
}

void HardwareSerial::printHex(const uint8_t b) {
	char x = (b >> 4) | '0';
	if (x > '9')
	x += 7;
	write(x);
	x = (b & 0x0F) | '0';
	if (x > '9')
	x += 7;
	write(x);
}

void HardwareSerial::printHex(const uint16_t w, bool swaporder) {
	uint8_t *ptr = (uint8_t *) &w;
	if (swaporder) {
		printHex(*(ptr++));
		printHex(*(ptr));
	} else {
		printHex(*(ptr + 1));
		printHex(*(ptr));
	}
}

void HardwareSerial::printHex(const uint32_t l, bool swaporder) {
	uint8_t *ptr = (uint8_t *) &l;
	if (swaporder) {
		printHex(*(ptr++));
		printHex(*(ptr++));
		printHex(*(ptr++));
		printHex(*(ptr));
	} else {
		ptr+=3;
		printHex(*(ptr--));
		printHex(*(ptr--));
		printHex(*(ptr--));
		printHex(*(ptr));
	}
}

uint8_t * HardwareSerial::printHex(uint8_t* p, uint8_t len, char sep) {
	for (uint8_t i = 0; i < len; i++) {
		if (sep && i) write(sep);
		printHex(*p++);
	}
	return p;
}

uint16_t * HardwareSerial::printHex(uint16_t* p, uint8_t len, char sep, bool swaporder) {
	for (uint8_t i = 0; i < len; i++) {
		if (sep && i) write(sep);
		printHex(*p++, swaporder);
	}
	return p;
}
