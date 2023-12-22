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

#include "config.h"
#include <stdlib.h>
#include <unistd.h>
#include <esd.h>

#include "defines.h"
#include "esound.h"

EsoundPlayer::EsoundPlayer(Copl *nopl, unsigned char bits, int channels,
			   int freq, const char *url)
  : EmuPlayer(nopl, bits, channels, freq, ESD_BUF_SIZE)
{
  socket = esd_play_stream((bits == 16 ? ESD_BITS16 : ESD_BITS8) |
			   (channels == 2 ? ESD_STEREO : ESD_MONO) |
			   ESD_STREAM | ESD_PLAY, freq, url, ADPLAY_VERSION);

  // on error, just exit here. esd_play_stream() does perror() already...
  if(socket < 0) exit(EXIT_FAILURE);
}

EsoundPlayer::~EsoundPlayer()
{
  close(socket);
}

void EsoundPlayer::output(const void *buf, unsigned long size)
{
  write(socket, buf, size);
}
