#include "Serial0.h"

#if (!USE_SOFTWARE_SERIAL && !DISABLE_UART && !DISABLE_UART0)
#include "HardwareSerial.h"
/* We rely on the normalized register names from HardwareSerial.h */
#if defined(UBRR0H) || defined(LINBRRH)
#if !defined(USART0_UDRE_vect) && !defined(LIN_TC_vect)
#error "Don't know what the Data Register Empty vector is called for the first UART"
#endif
ring_buffer rx_buffer  =  { { 0 }, 0, 0 };
/*ring_buffer tx_buffer  =  { { 0 }, 0, 0 }; Our code doesn't use TX so this saves memory
#if defined(USART0_UDRE_vect)
ISR(USART0_UDRE_vect) {
	if (tx_buffer.head == tx_buffer.tail) {
		// Buffer empty, so disable interrupts]
		UCSR0B &= ~(1 << UDRIE);
		} else {
		// There is more data in the output buffer. Send the next byte
		unsigned char c = tx_buffer.buffer[tx_buffer.tail];
		tx_buffer.tail = (tx_buffer.tail + 1) % SERIAL_BUFFER_SIZE;
		UDR0 = c;
	}
}
#endif*/
#endif
ISR(USART0_RX_vect) {
	unsigned char c  =  UDR0;
	store_char(c, &rx_buffer);
}

HardwareSerial Serial(&rx_buffer, &rx_buffer, &UBRR0H, &UBRR0L, &UCSR0A, &UCSR0B, &UCSR0C, &UDR0);

#endif