/*
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
  TlcMux_init();
}

uint8_t color;
void loop()
{
  TlcMux_clear();
  for (uint8_t col = 0; col < 11; col++) {
    for (uint8_t row = 0; row < NUM_ROWS; row++) {
      TlcMux_set(row, col + color * 16, 4095);
    }
    color++;
    if (color == 3) {
      color = 0;
    }
  }
  delay(2000);
}
