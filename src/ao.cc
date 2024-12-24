/*
 * AdPlay/UNIX - OPL2 audio player
 * Copyright (C) 2001 - 2003, 2024 Simon Peter <dn.tlp@gmx.net>
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

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <ao/ao.h>

#include "ao.h"

AOPlayer::AOPlayer(Copl *nopl, const char *device, unsigned char bits,
		     int channels, int freq, unsigned long bufsize)
  : EmuPlayer(nopl, bits, channels, freq, bufsize)
{
  ao_sample_format format = {0};
  int default_driver;

  ao_initialize();

  default_driver = ao_default_driver_id();

  format.bits = bits;
  format.channels = channels;
  format.rate = freq;
  format.byte_format = AO_FMT_NATIVE;

  aodevice = ao_open_live(default_driver, &format, NULL);
}

AOPlayer::~AOPlayer()
{
  // We're skipping this here, even though we should call ao_close()
  // on exit. Unfortunately, the pulseaudio backend in ao has a race
  // condition where it fails if we call ao_close() while ao_play() is
  // running. This happens easily due to adplay's use of SIGINT to
  // exit and ao uses multiple threads for playback.

  // if(aodevice != NULL) {
  //   ao_close(aodevice);
  // }

  // ao_shutdown();
}

void AOPlayer::output(const void *buf, unsigned long size)
{
  ao_play(aodevice, (char *)buf, (uint_32)size); 
}
