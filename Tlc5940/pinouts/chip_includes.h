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

#ifndef TLC_CHIP_INCLUDES_H
#define TLC_CHIP_INCLUDES_H

/** \file
    Includes the chip-specfic defaults and pin definitions. */

/* Chip Specific Pinouts */
#if defined (__AVR_ATmega168__)  \
 || defined (__AVR_ATmega168P__) \
 || defined (__AVR_ATmega88P__)  \
 || defined (__AVR_ATmega88__)   \
 || defined (__AVR_ATmega48P__)  \
 || defined (__AVR_ATmega48__)   \
 || defined (__AVR_ATmega328P__)

/* Diecimila / Duemilanove / almost everything */
#include "ATmega_xx8.h"

#elif defined (__AVR_ATmega8__)

/* ATmega8 */
#include "ATmega_8.h"

#elif defined (__AVR_ATmega164P__) \
   || defined (__AVR_ATmega324P__) \
   || defined (__AVR_ATmega644__)  \
   || defined (__AVR_ATmega644P__)

/* Sanguino */
#include "ATmega_xx4.h"

#else
#error "Unknown Chip!"
#endif

#endif