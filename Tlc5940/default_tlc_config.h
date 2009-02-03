/*
 * Copyright (c) 2008 by Alex Leone <acleone ~AT~ gmail.com>
 * Last Edited: 2008-11-26
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TLC_CONFIG_H
#define TLC_CONFIG_H

#include <stdint.h>

/**
 * \file
 * Configuration for the Arduino Tlc5940 library.  After making changes to this
 * file, delete Tlc5940.o in this folder so the changes are applied.
 *
 * A summary of all the options:
 * - Number of TLCs daisy-chained: #NUM_TLCS (default 1)
 * - Enable/Disable VPRG functionality: #VPRG_ENABLED (default 0)
 * - Enable/Disable XERR functionality: #XERR_ENABLED (default 0)
 * - Should the library use bit-banging (any pins) or hardware SPI (faster): #DATA_TRANSFER_MODE (default TLC_SPI)
 * - Which pins to use for bit-banging: #SIN_PIN, #SIN_PORT, #SIN_DDR and #SCLK_PIN, #SCLK_PORT, #SCLK_DDR
 * - The PWM period: #TLC_PWM_PERIOD (be sure to change #TLC_GSCLK_PERIOD accordingly!)
 *
 * How to change the pin mapping:
 * - Arduino digital pin 0-7  = PORTD, PD0-7
 * - Arduino digital pin 8-13 = PORTB, PB0-5
 * - Arduino analog pin  0-5  = PORTC, PC0-5
 *
 */

/**
 * Number of TLCs daisy-chained.  To daisy-chain, attach the SOUT (TLC pin 17) of the first TLC
 * to the SIN (TLC pin 26) of the next.  The rest of the pins are attached normally.
 * \note Each TLC needs it's own IREF resistor
 */
#define NUM_TLCS	1

/**
 * If more than 16 TLCs are daisy-chained, the channel type has to be uint16_t.
 * Default is uint8_t (16 TLC's max).
 * (uint8_t has a max of 255).
 */
#define TLC_CHANNEL_TYPE	uint8_t

/**
 * Enables/disables VPRG (TLC pin 27) functionality.  If you need to set dot correction data,
 * this needs to be enabled.
 * - VPRG is not connected.  <em>TLC pin 27 must be grounded!</em> (default)
 * - VPRG is connected
 * \note VPRG to GND inputs grayscale data, VPRG to Vcc inputs dot-correction data
 */
#define VPRG_ENABLED	0

#if VPRG_ENABLED
/** PD6 (Arduino digitial pin 6) -> VPRG (TLC pin 27) */
#define VPRG_PIN	PD6
#define VPRG_PORT	PORTD
#define VPRG_DDR	DDRD
#endif

/**
 * Enables/disables XERR (TLC pin 16) functionality to check for shorted/broken LEDs
 * - XERR is not connected (default)
 * - XERR is connected
 * \note XERR is active low
 */
#define XERR_ENABLED	0

#if XERR_ENABLED
/** PD6 (Arduino digital pin 5) -> XERR (TLC pin 16) */
#define XERR_PIN	PD5
#define XERR_PORT	PORTD
#define XERR_DDR	DDRD
#define XERR_PINS	PIND
#endif


/** Bit-bang using any two i/o pins */
#define TLC_BITBANG		1
/** Use the much faster hardware SPI module */
#define TLC_SPI			2

/**
 * Determines how data should be transfered to the TLCs.  Bit-banging can use
 * any two i/o pins, but the hardware SPI is faster.
 * - Bit-Bang = TLC_BITBANG
 * - Hardware SPI = TLC_SPI (default)
 */
#define DATA_TRANSFER_MODE	TLC_SPI


#if DATA_TRANSFER_MODE == TLC_BITBANG

/** PB0 (Arduino digital pin 8) -> SIN (TLC pin 26) */
#define SIN_PIN		PB0
#define SIN_PORT	PORTB
#define SIN_DDR		DDRB

/** PD7 (Arduino digital pin 7) -> SCLK (TLC pin 25) */
#define SCLK_PIN	PD7
#define SCLK_PORT	PORTD
#define SCLK_DDR	DDRD

#elif DATA_TRANSFER_MODE == TLC_SPI

/** MOSI (Arduino digital pin 11) -> SIN (TLC pin 26) */
#define TLC_MOSI_PIN	PB3
#define TLC_MOSI_PORT	PORTB
#define TLC_MOSI_DDR	DDRB

/** SCK (Arduino digital pin 13) -> SCLK (TLC pin 25) */
#define	TLC_SCK_PIN		PB5
#define TLC_SCK_PORT	PORTB
#define TLC_SCK_DDR		DDRB

/*
 * SS this will be set to output as to not interfere with SPI master operation.
 * This pin will not be used by the library other than setting it to output.
 */
#define TLC_SS_PIN		PB2
#define TLC_SS_DDR		DDRB

#define SIN_PIN		TLC_MOSI_PIN
#define SIN_PORT	TLC_MOSI_PORT
#define SIN_DDR		TLC_MOSI_DDR

#define SCLK_PIN	TLC_SCK_PIN
#define SCLK_PORT	TLC_SCK_PORT
#define SCLK_DDR	TLC_SCK_DDR

#else
#error "Invalid DATA_TRANSFER_MODE specified, see DATA_TRANSFER_MODE"
#endif

/*
 * The pins below are device-specific.  Different chips may have different
 * pins for Timer1 and Timer2.
 */

/** OC1A (Arduino digital pin 9) -> XLAT (TLC pin 24) */
#define XLAT_PIN	PB1
#define XLAT_PORT	PORTB
#define XLAT_DDR	DDRB

/** OC1B (Arduino digital pin 10) -> BLANK (TLC pin 23) */
#define BLANK_PIN	PB2
#define BLANK_PORT	PORTB
#define BLANK_DDR	DDRB

/** OC2B (Arduino digital pin 3) -> GSCLK (TLC pin 18) */
#define GSCLK_PIN	PD3
#define GSCLK_PORT	PORTD
#define GSCLK_DDR	DDRD

/**
 * Determines how long each PWM period should be, in clocks.
 * \f$\displaystyle f_{PWM} = \frac{f_{osc}}{2 * TLC\_PWM\_PERIOD} Hz \f$
 * This is related to TLC_GSCLK_PERIOD:
 * \f$\displaystyle TLC\_PWM\_PERIOD = \frac{(TLC\_GSCLK\_PERIOD + 1) * 4096}{2} \f$
 * \note The default of 8192 means the PWM frequency is 976.5625Hz
 */
#define TLC_PWM_PERIOD		8192

/**
 * Determines how long each GSCLK pulse is.
 * This is related to TLC_PWM_PERIOD:
 * \f$\displaystyle TLC\_GSCLK\_PERIOD = \frac{2 * TLC\_PWM\_PERIOD}{4096} - 1 \f$
 * \note Default is 3
 */
#define	TLC_GSCLK_PERIOD	3


/*
 * Various Macros
 */

/**
 * Arranges 2 grayscale values (0 - 4095) in the packed array format
 */
#define GS_DUO(a, b)	((a) >> 4), ((a) << 4) | ((b) >> 8), (b)


#if VPRG_ENABLED
/**
 * Arranges 4 dot correction values (0 - 63) in the packed array format.
 * \see setDCtoProgmemArray
 */
#define DC_QUARTET(a, b, c, d)	((a) << 2) | ((b) >> 4), ((b) << 4) | ((c) >> 2), ((c) << 6) | (d)
#endif

/*
 * Basic Pin setup:
 * ------------                                             ---u----
 * ARDUINO   13|-> SCLK (pin 25)                      OUT1 |1     28| OUT channel 0
 *           12|                                      OUT2 |2     27|-> GND (VPRG)
 *           11|-> SIN (pin 26)                       OUT3 |3     26|-> SIN (pin 11)
 *           10|-> BLANK (pin 23)                     OUT4 |4     25|-> SCLK (pin 13)
 *            9|-> XLAT (pin 24)                        .  |5     24|-> XLAT (pin 9)
 *            8|                                        .  |6     23|-> BLANK (pin 10)
 *            7|                                        .  |7     22|-> GND
 *            6|                                        .  |8     21|-> VCC (+5V)
 *            5|                                        .  |9     20|-> 2K Resistor -> GND
 *            4|                                        .  |10    19|-> +5V (DCPRG)
 *            3|-> GSCLK (pin 18)                       .  |11    18|-> GSCLK (pin 3)
 *            2|                                        .  |12    17|-> SOUT (if daisy-chained, this goes to SIN of the next TLC)
 *            1|                                        .  |13    16|-> XERR
 *            0|                                      OUT14|14    15| OUT channel 15
 * ------------                                             --------
 *
 * -  Put the longer leg (anode) of the LEDs in the +5V and the shorter leg (cathode) in OUT(0-15).
 * -  +5V from Arduino -> TLC pin 21 and 19     (VCC and DCPRG)
 * -  GND from Arduino -> TLC pin 22 and 27     (GND and VPRG)
 * -  digital 3        -> TLC pin 18            (GSCLK)
 * -  digital 9        -> TLC pin 24            (XLAT)
 * -  digital 10       -> TLC pin 23            (BLANK)
 * -  digital 11       -> TLC pin 26            (SIN)
 * -  digital 13       -> TLC pin 25            (SCLK)
 * -  The 2K resistor between TLC pin 20 and GND will let ~20mA through each LED.
 *
 * If you are daisy-chaining more than one TLC, connect the SOUT of the first TLC to
 * the SIN of the next.  All the other pins should just be connected together:
 *   BLANK of TLC1 -> BLANK of TLC2 -> ...
 * The one exception is that each TLC needs it's own resistor between pin 20 and GND.
 */

#endif