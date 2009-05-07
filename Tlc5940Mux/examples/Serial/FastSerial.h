#ifndef SERIAL_H
#define SERIAL_H

#ifndef SERIAL_BAUD
#define SERIAL_BAUD     115200L
#endif

uint8_t rxBuffer[256];
uint8_t rxBufferStart;
volatile uint8_t rxBufferEnd;

ISR(USART_RX_vect)
{
    rxBuffer[rxBufferEnd++] = UDR0;        
}

static uint8_t serial_available()
{
    return rxBufferEnd - rxBufferStart;
}

static uint8_t serial_read()
{
    return rxBuffer[rxBufferStart++];
}


static void serial_init()
{
#if defined(__AVR_ATmega8__)
	UBRRH = ((F_CPU / 16 + SERIAL_BAUD / 2) / SERIAL_BAUD - 1) >> 8;
	UBRRL = ((F_CPU / 16 + SERIAL_BAUD / 2) / SERIAL_BAUD - 1);
    UCSRB = _BV(RXEN) | _BV(TXEN) | _BV(RXCIE);
#else
	//UBRR0H = ((F_CPU / 16 + SERIAL_BAUD / 2) / SERIAL_BAUD - 1) >> 8;
	//UBRR0L = ((F_CPU / 16 + SERIAL_BAUD / 2) / SERIAL_BAUD - 1);
	UBRR0H = 0;
	UBRR0L = 3; // 500k w/ 2x bit for 0% error
	UCSR0A = _BV(U2X0);
	UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);
#endif
}

static void serial_write(uint8_t c)
{
#if defined(__AVR_ATmega8__)
	while (!(UCSRA & _BV(UDRE)))
		;
	UDR = c;
#else
	while (!(UCSR0A & _BV(UDRE0)))
		;
	UDR0 = c;
#endif
}

#endif
