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

/*
 * players.h - This header conditionally includes AdPlay's output drivers
 * and sets a reasonable default.
 */

#ifndef H_PLAYERS
#define H_PLAYERS

#include "config.h"

// Enumerate ALL outputs (regardless of availability)
enum Outputs {none, null, oss, disk};

#define DEFAULT_DRIVER none

// Null (silent) output
#ifdef DRIVER_NULL
#include "null.h"
#undef DEFAULT_DRIVER
#define DEFAULT_DRIVER null
#endif

// Disk writer
#ifdef DRIVER_DISK
#include "disk.h"
#endif

// OSS driver
#ifdef DRIVER_OSS
#include "oss.h"
#undef DEFAULT_DRIVER
#define DEFAULT_DRIVER oss
#endif

#endif
