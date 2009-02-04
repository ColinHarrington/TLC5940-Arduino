/*  Copyright (c) 2009 by Alex Leone <acleone ~AT~ gmail.com>

    This file is part of the Arduino TLC5940 Library.

    The Arduino TLC5940 Library is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    The Arduino TLC5940 Library is distributed in the hope that it will be
    useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with The Arduino TLC5940 Library.  If not, see
    <http://www.gnu.org/licenses/>. */

#ifndef TLC_FADES_H
#define TLC_FADES_H

/** \file
    TLC fading functions. */

#include <avr/interrupt.h>

#include "Tlc5940.h"
#include "WProgram.h"

#define TLC_FADE_BUFFER_LENGTH    24

struct Tlc_Fade {
    TLC_CHANNEL_TYPE channel;
    int16_t startValue;
    int16_t changeValue;
    uint32_t startMillis;
    uint32_t endMillis;
} tlc_fadeBuffer[TLC_FADE_BUFFER_LENGTH];

uint8_t tlc_fadeBufferSize;

uint8_t tlc_updateFades();
uint8_t tlc_updateFades(uint32_t currentMillis);
uint8_t tlc_addFade(Tlc_Fade *fade);
uint8_t tlc_addFade(TLC_CHANNEL_TYPE channel, int16_t startValue,
                    int16_t endValue, uint32_t startMillis, uint32_t endMillis);
uint8_t tlc_isFading(TLC_CHANNEL_TYPE channel);
uint8_t tlc_removeFade(TLC_CHANNEL_TYPE channel);
static void tlc_removeFadeFromBuffer(Tlc_Fade *current, Tlc_Fade *end);

/**
 * \addtogroup ExtendedFunctions
 * \code #include "tlc_fades.h" \endcode
 * - tlc_addFade(TLC_CHANNEL_TYPE channel, int16_t startValue, int16_t endValue,
 *                            uint32_t startMillis, uint32_t endMillis)
 * - tlc_removeFade(TLC_CHANNEL_TYPE channel)
 */
/* @{ */

/** Adds a fade to the buffer. */
uint8_t tlc_addFade(Tlc_Fade *fade)
{
    if (tlc_fadeBufferSize == TLC_FADE_BUFFER_LENGTH) {
        return 0; // fade buffer full
    }
    Tlc_Fade *p = tlc_fadeBuffer + tlc_fadeBufferSize++;
    p->channel = fade->channel;
    p->startValue = fade->startValue;
    p->changeValue = fade->changeValue;
    p->startMillis = fade->startMillis;
    p->endMillis = fade->endMillis;
    return tlc_fadeBufferSize;
}

/**
 * Adds a fade to the fade buffer.
 * \param channel the ouput channel this fade is on
 * \param startValue the value at the start of the fade
 * \param endValue the value at the end of the fade
 * \param startMillis the millis() when to start the fade
 * \param endMillis the millis() when to end the fade
 * \returns 1 if added successfully, 0 if the fade buffer is full.
 */
uint8_t tlc_addFade(TLC_CHANNEL_TYPE channel, int16_t startValue,
                    int16_t endValue, uint32_t startMillis, uint32_t endMillis)
{
    if (tlc_fadeBufferSize == TLC_FADE_BUFFER_LENGTH) {
        return 0; // fade buffer full
    }
    Tlc_Fade *p = tlc_fadeBuffer + tlc_fadeBufferSize++;
    p->channel = channel;
    p->startValue = startValue;
    p->changeValue = endValue - startValue;
    p->startMillis = startMillis;
    p->endMillis = endMillis;
    return tlc_fadeBufferSize;
}


uint8_t tlc_isFading(TLC_CHANNEL_TYPE channel)
{
    Tlc_Fade *end = tlc_fadeBuffer + tlc_fadeBufferSize;
    for (Tlc_Fade *p = tlc_fadeBuffer; p < end; p++) {
        if (p->channel == channel) {
            return 1;
        }
    }
    return 0;
}

/**
 * Removes any fades from the fade buffer on this channel.
 * \param channel which channel the fades are on
 * \returns how many fades were removed
 */
uint8_t tlc_removeFade(TLC_CHANNEL_TYPE channel)
{
    uint8_t removed = 0;
    Tlc_Fade *end = tlc_fadeBuffer + tlc_fadeBufferSize;
    for (Tlc_Fade *p = tlc_fadeBuffer; p < end; p++) {
        if (p->channel == channel) {
            removed++;
            tlc_removeFadeFromBuffer(p, --end);
        }
    }
    return removed;
}

/**
 * Copies the end of the buffer to the current and decrements
 * tlc_fadeBufferSize.  This will change the end of the buffer (pass by
 * reference)
 * \param current the fade to be removed
 * \param endp the end of the fade buffer (pointer to pointer)
 */
static void tlc_removeFadeFromBuffer(Tlc_Fade *current, Tlc_Fade *endp)
{
    if (endp != current) { // if this is not the last fade
        current->channel = endp->channel;
        current->startValue = endp->startValue;
        current->changeValue = endp->changeValue;
        current->startMillis = endp->startMillis;
        current->endMillis = endp->endMillis;
    }
    tlc_fadeBufferSize--;
}

/** Updates fades using millis() */
uint8_t tlc_updateFades()
{
    return tlc_updateFades(millis());
}

/** Updates any running fades.
    \param currentMillis the current millis() time.
    \returns 0 if there are no fades left in the buffer. */
uint8_t tlc_updateFades(uint32_t currentMillis)
{
    Tlc_Fade *end = tlc_fadeBuffer + tlc_fadeBufferSize;
    uint8_t needsUpdate = 0;
    for (Tlc_Fade *p = tlc_fadeBuffer; p < end; p++){
        if (currentMillis >= p->endMillis) { // fade done
            Tlc.set(p->channel, p->startValue + p->changeValue);
            needsUpdate = 1;
            tlc_removeFadeFromBuffer(p, --end);
        } else {
            uint32_t startMillis = p->startMillis;
            if (currentMillis >= startMillis) {
                Tlc.set(p->channel, p->startValue + p->changeValue
                        * (int32_t)(currentMillis - startMillis)
                        / (int32_t)(p->endMillis - startMillis));
                needsUpdate = 1;
            }
        }
    }
    if (needsUpdate) {
        Tlc.update();
    }
    return tlc_fadeBufferSize;
}

/* @} */

#endif