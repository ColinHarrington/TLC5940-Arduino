/*
 * Fades 5 random outputs randomly.
 *
 * See the BasicUse example for hardware setup.
 *
 * Alex Leone <acleone ~AT~ gmail.com>, 2008-11-12
 */

#include "Tlc5940.h"
#include "tlc_fades.h"

void setup()
{
  Tlc.init();
}

void loop()
{
  while (tlc_fadeBufferSize < 10) {

    TLC_CHANNEL_TYPE channel = random(NUM_TLCS * 16);

    // remove any fades on this channel already happening
    tlc_removeFade(channel);

    int startValue = Tlc.get(channel);
    int endValue = random(4095);

    unsigned long currentMillis = millis();
    // Start the fade anywhere from 30 to 3030 milliseconds from now
    unsigned long startMillis = currentMillis + 30 + random(3000);
    // End the fade after 100 to 3100 milliseconds
    unsigned long endMillis = startMillis + 100 + random(3000);

    // Add the fade to the buffer.
    // The default buffer length is 16, so a max of 16 fades at once.
    // tlc_addFade will return 0 if the fade buffer is full.
    // If you need more fades, edit TLC_FADE_BUFFER_LENGTH in tlc_fades.h
    tlc_addFade(channel, startValue, endValue, startMillis, endMillis);
  }
  // call this to update the fades
  tlc_updateFades(millis());
}
