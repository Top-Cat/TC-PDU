#include "Serial0.h"

#include "HardwareSerial.h"

/* We rely on the normalized register names from HardwareSerial.h */
ring_buffer rx_buffer = { { 0 }, 0, 0 };

ISR(USART0_RX_vect) {
	unsigned char c = UDR0;
	store_char(c, &rx_buffer);
}

HardwareSerial Serial(&rx_buffer, &rx_buffer, &UBRR0H, &UBRR0L, &UCSR0A, &UCSR0B, &UCSR0C, &UDR0);
