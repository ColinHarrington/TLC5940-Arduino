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

#include "Tlc5940Mux.h"
#include "tlc_shift8.h"

volatile uint8_t isShifting;
uint8_t row;

ISR(TIMER1_OVF_vect)
{
  if (!isShifting) {
    disable_XLAT_pulses();
    //XLAT_PORT |=  _BV(XLAT_PIN);
    //XLAT_PORT &= ~_BV(XLAT_PIN);
    PORTC = row;
    if (++row == TLC_NUM_MUX) {
      row = 0;
    }
    isShifting = 1;
    sei();
    uint8_t *p = tlcMux_GSData[row];
    uint8_t * const end = p + NUM_TLCS * 24;
    while (p < end) {
      tlc_shift8(*p++);
      tlc_shift8(*p++);
      tlc_shift8(*p++);
    }
    enable_XLAT_pulses();
    isShifting = 0;
  }
}

void setup()
{
  DDRC |= _BV(PC0) | _BV(PC1) | _BV(PC2);
  tlc_shift8_init();
  TlcMux.init();
  for (uint8_t i = 0; i < 8; i++) {
    TlcMux.set(i, i, 4095);
  }
}

void loop()
{
  
}
