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

#define TLC_FADE_BUFFER_LENGTH    24

struct Tlc_Fade {
    TLC_CHANNEL_TYPE channel;
    int16_t startValue;
    int16_t changeValue;
    uint32_t startMillis;
    uint32_t endMillis;
} tlc_fadeBuffer[TLC_FADE_BUFFER_LENGTH];

volatile uint8_t tlc_fadeBufferSize;
volatile uint8_t tlc_alreadyCheckingFadeBuffer;

//volatile void tlc_fadeBufferCheckCallback(void);
uint8_t tlc_updateFades(uint32_t currentMillis);
uint8_t tlc_addFade(TLC_CHANNEL_TYPE channel, int16_t startValue,
                 int16_t endValue, uint32_t startMillis, uint32_t endMillis);
uint8_t tlc_removeFade(TLC_CHANNEL_TYPE channel);
void tlc_removeFadeFromBuffer(Tlc_Fade *current, Tlc_Fade **end);

/**
 * \addtogroup ExtendedFunctions
 * \code #include "tlc_fades.h" \endcode
 * - tlc_addFade(TLC_CHANNEL_TYPE channel, int16_t startValue, int16_t endValue,
 *                            uint32_t startMillis, uint32_t endMillis)
 * - tlc_removeFade(TLC_CHANNEL_TYPE channel)
 */
/* @{ */

/**
 * Adds a fade to the fade buffer.
 * \param channel the ouput channel this fade is on
 * \param startValue the value at the start of the fade
 * \param endValue the value at the end of the fade
 * \param startMillis the millis() when to start the fade
 * \param endMillis the millis() when to end the fade
 * \returns 1 if added successfully, 0 if the fade buffer is full.
 */
/*uint8_t tlc_addFade(TLC_CHANNEL_TYPE channel, int16_t startValue,
                 int16_t endValue, uint32_t startMillis, uint32_t endMillis)
{
    if (tlc_fadeBufferSize == TLC_FADE_BUFFER_LENGTH) {
        return 0; // fade buffer full
    }
    // disable interrupts so we add at the actual end of the buffer
    uint8_t oldSREG = SREG;
    cli();
    Tlc_Fade p = tlc_fadeBuffer[tlc_fadeBufferSize++];
    p.channel = channel;
    p.startValue = startValue;
    p.changeValue = endValue - startValue;
    p.startMillis = startMillis;
    p.endMillis = endMillis;
    tlc_onUpdateFinished = tlc_fadeBufferCheckCallback;
    SREG = oldSREG;
    tlc_fadeBufferCheckCallback();
}*/

uint8_t tlc_addFade(TLC_CHANNEL_TYPE channel, int16_t startValue,
                 int16_t endValue, uint32_t startMillis, uint32_t endMillis)
{
    if (tlc_fadeBufferSize == TLC_FADE_BUFFER_LENGTH) {
        return 0; // fade buffer full
    }
    tlc_fadeBufferSize++;
    Tlc_Fade *p = tlc_fadeBuffer + tlc_fadeBufferSize;
    p->channel = channel;
    p->startValue = startValue;
    p->changeValue = endValue - startValue;
    p->startMillis = startMillis;
    p->endMillis = endMillis;
}

/**
 * Removes any fades from the fade buffer on this channel.
 * \param channel which channel the fades are on
 * \returns how many fades were removed
 */
uint8_t tlc_removeFade(TLC_CHANNEL_TYPE channel)
{
    // disable interrupts so we don't remove a fade while an update is in
    // progess
    uint8_t oldSREG = SREG;
    cli();
    uint8_t removed = 0;
    Tlc_Fade *p = tlc_fadeBuffer;
    Tlc_Fade *end = tlc_fadeBuffer + tlc_fadeBufferSize;
    for (; p < end; p++) {
        if (p->channel == channel) {
            removed++;
            tlc_removeFadeFromBuffer(p, &end);
        }
    }
    SREG = oldSREG;
    return removed;
}

/**
 * Copies the end of the buffer to the current and decrements
 * tlc_fadeBufferSize.  This will change the end of the buffer (pass by
 * reference)
 * \param current the fade to be removed
 * \param endp the end of the fade buffer (pointer to pointer)
 */
void tlc_removeFadeFromBuffer(Tlc_Fade *current, Tlc_Fade **endp)
{
    Tlc_Fade *end = *endp;
    if (--end != current) { // if this is not the last fade
        // TODO: switch to memcpy?
        current->channel = end->channel;
        current->startValue = end->startValue;
        current->changeValue = end->changeValue;
        current->startMillis = end->startMillis;
        current->endMillis = end->endMillis;
    }
    tlc_fadeBufferSize--;
}

/**
 * This is called every PWM period to do stuff.
 */
/*volatile void tlc_fadeBufferCheckCallback(void)
{
    if (tlc_alreadyCheckingFadeBuffer)
        return;
    tlc_alreadyCheckingFadeBuffer = 1;
    Tlc_Fade *p = tlc_fadeBuffer;
    Tlc_Fade *end = tlc_fadeBuffer + tlc_fadeBufferSize;
    uint32_t currentMillis = millis();
    uint8_t needsUpdate = 0;
    for (; p < end; p++){
        if (currentMillis >= p->endMillis) { // fade done
            Tlc.set(p->channel, p->startValue + p->changeValue);
            needsUpdate = 1;
            tlc_removeFadeFromBuffer(p, &end);
        } else {
            uint32_t startMillis = p->startMillis;
            if (currentMillis >= startMillis) {
                Tlc.set(p->channel, p->startValue + p->changeValue
                        * (currentMillis - startMillis)
                        / (p->endMillis - startMillis));
                needsUpdate = 1;
            }
        }
    }
    if (needsUpdate) {
        Tlc.update();
    }
    if (tlc_fadeBufferSize == 0) { // all fades have been removed
        tlc_onUpdateFinished = 0; // we don't need to call this function anymore
    } else if (!needsUpdate) {
        // continue PWM updates
        set_XLAT_interrupt();
    }
    tlc_alreadyCheckingFadeBuffer = 0;
}*/

uint8_t tlc_updateFades(uint32_t currentMillis)
{
    if (tlc_needXLAT) {
        return 1;
    }
    Tlc_Fade *p = tlc_fadeBuffer;
    Tlc_Fade *end = tlc_fadeBuffer + tlc_fadeBufferSize;
    uint8_t needsUpdate = 0;
    for (; p < end; p++){
        if (currentMillis >= p->endMillis) { // fade done
            Tlc.set(p->channel, p->startValue + p->changeValue);
            needsUpdate = 1;
            tlc_removeFadeFromBuffer(p, &end);
        } else {
            uint32_t startMillis = p->startMillis;
            if (currentMillis >= startMillis) {
                Tlc.set(p->channel, p->startValue + p->changeValue
                        * (currentMillis - startMillis)
                        / (p->endMillis - startMillis));
                needsUpdate = 1;
            }
        }
    }
    if (needsUpdate) {
        Tlc.update();
        return 1;
    }
    return tlc_fadeBufferSize;
}


/* @} */

#endif
