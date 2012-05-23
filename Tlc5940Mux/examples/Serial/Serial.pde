/*
    See Tlc5940Mux/tlc5940mux_circuit_example.png for an example circuit.

    Analog 0-2 (PORTC) is hooked to a 3:8 line decoder (74LS138) which pull the
    Base of a PNP darlington (MPSA63) low through a 10k resistor for each row.
    +5V is connected to the Emitter of the PNP's, and all the anodes (+) of the
    leds for each row are connected to the Collector.
    
    Currently I'm trying to fix timing issues by putting some D-type positive
    edge triggered latches between PORTC and the 3:8 line decoder, clocked on
    the rising edge of XLAT, but this doesn't seem to help.  I'm bringing the
    circuit in to the logic analyzer this weekend to see exactly when the ISR
    runs and how long it takes to shift in the GS data.
    
    Alex Leone, 2009-04-30
*/

#define  NUM_TLCS  3
#define  NUM_ROWS  8
#include "Tlc5940Mux.h"

#define SERIAL_BAUD  57600L
#include "FastSerial.h"

#define  SERIAL_VERSION 'a'

volatile uint8_t isShifting;
uint8_t shiftRow;

ISR(TIMER1_OVF_vect)
{
  if (!isShifting) {
    disable_XLAT_pulses();
    isShifting = 1;
    sei();
    TlcMux_shiftRow(shiftRow);
    PORTC = shiftRow++;
    if (shiftRow == NUM_ROWS) {
      shiftRow = 0;
    }
    enable_XLAT_pulses();
    isShifting = 0;
  }
}

void setup()
{
  DDRC |= _BV(PC0) | _BV(PC1) | _BV(PC2);
  TIMSK0 = 0; // turn off millis()
  serial_init();
  TlcMux_init();
}

void loop()
{
  if (serial_available()) {
    uint8_t c = serial_read();
    switch (c) {
      case 'm':
      {
        while (!serial_available())
          ;
        uint8_t row = serial_read();
        uint8_t *p = tlcMux_GSData[row];
        uint8_t * const end = p + NUM_TLCS * 24;
        while (p < end) {
          while (!serial_available())
            ;
          *p++ = serial_read();
        }
      }
        break;
      case 'M':
      {
        while (serial_available() < 4)
          ;
        uint8_t *p = tlcMux_GSData[0] + (uint16_t)((serial_read() << 8)
                                                   | serial_read());
        uint8_t * const end = p + (uint16_t)((serial_read() << 8)
                                                   | serial_read());
        while (p < end) {
          while (!serial_available())
            ;
          *p++ = serial_read();
        }
      }
        break;
      case 'C':
        TlcMux_clear();
        break;
      case 'c':
        while (!serial_available())
          ;
        TlcMux_clearRow(serial_read());
        break;
      case 'a':
        break;
      case 'i':
        serial_write(SERIAL_VERSION);
        serial_write(NUM_TLCS);
        serial_write(NUM_ROWS);
        serial_write(TLC_CHANNEL_TYPE_STR);
        break;
      case 's':
      {
        #if TLC_CHANNEL_TYPE_STR == '1'
        while (serial_available() < 4)
          ;
        uint8_t row = serial_read();
        uint8_t channel = serial_read();
        #else
        while (serial_available() < 5)
          ;
        uint8_t row = serial_read();
        uint16_t channel = serial_read() << 8 | serial_read();
        #endif
        uint16_t value = serial_read() << 8 | serial_read();
        TlcMux_set(row, channel, value);
      }
        break;
      case 'S':
      {
        while (!serial_available())
          ;
        uint8_t row = serial_read();
        for (TLC_CHANNEL_TYPE channel = 0; channel < NUM_TLCS * 16; channel++) {
          while (serial_available() < 2)
            ;
          uint16_t value = serial_read() << 8 | serial_read();
          TlcMux_set(row, channel, value);
        }
      }
        break;
      case 't':
      {
        while (serial_available() < 2)
          ;
        uint16_t value = serial_read() << 8 | serial_read();
        TlcMux_setAll(value);
      }
        break;
      case 'T':
      {
        while (serial_available() < 3)
          ;
        uint8_t row = serial_read();
        uint16_t value = serial_read() << 8 | serial_read();
        TlcMux_setRow(row, value);
      }
        break;
      case 'g':
      {
        #if TLC_CHANNEL_TYPE_STR == '1'
        while (serial_available() < 2)
          ;
        uint8_t row = serial_read();
        uint8_t channel = serial_read();
        #else
        while (serial_available() < 3)
          ;
        uint8_t row = serial_read();
        uint16_t channel = serial_read() << 8 | serial_read();
        #endif
        uint16_t result = TlcMux_get(row, channel);
        serial_write((uint8_t)(result >> 8));
        serial_write((uint8_t)(result));
      } 
        break;
      case 'G':
      { 
        while (!serial_available())
          ;
        uint8_t row = serial_read();
        for (TLC_CHANNEL_TYPE channel = 0; channel < NUM_TLCS * 16; channel++) {
          uint16_t result = TlcMux_get(row, channel);
          serial_write((uint8_t)(result >> 8));
          serial_write((uint8_t)(result));
        }
      } 
        break;
      default:
        serial_write('e');
        break; 
    }
    serial_write(c);
  }
}
