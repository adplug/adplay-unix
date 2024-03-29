/*
 * AdPlay/UNIX - OPL2 audio player
 * Copyright (C) 2001, 2002 Simon Peter <dn.tlp@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  
 */

#ifndef H_DEFINES
#define H_DEFINES

#include "config.h"

/***** Defines *****/

// AdPlay/UNIX version string
#define ADPLAY_VERSION "AdPlay/UNIX " VERSION

// Message urgency levels
#define MSG_PANIC	0	// Unmaskable
#define MSG_ERROR	1
#define MSG_WARN	2
#define MSG_NOTE	3
#define MSG_DEBUG	4

#ifndef MIN
#  define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#  define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

/***** Global functions *****/

void message(int level, const char *fmt, ...);

#endif
