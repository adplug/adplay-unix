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

#ifndef H_SDL
#define H_SDL

#include "output.h"

#include "SDL.h"
#include "SDL_mutex.h"
#include "SDL_thread.h"

class SDLPlayer: public EmuPlayer
{
public:
  SDLPlayer(unsigned char bits, int channels, int freq, unsigned long bufsize);
  virtual ~SDLPlayer();

protected:
  virtual void output(const void *buf, unsigned long size);

private:
  volatile int	DataReady;
  unsigned char	*playbuf;
  unsigned long	playsize, played;

  static void callback(void *userdata, Uint8 *stream, int len);
};

#endif
