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

#include "defines.h"
#include "sdl.h"

SDLPlayer::SDLPlayer(unsigned char bits, int channels, int freq)
  : EmuPlayer(bits, channels, freq, SDL_BUFFER_SIZE)
{
   SDL_AudioSpec spec;
   
   memset(&spec, 0x00, sizeof(SDL_AudioSpec));

   if (SDL_Init(SDL_INIT_AUDIO) < 0)
   {
      message(MSG_ERROR, "unable to initialize SDL -- %s", SDL_GetError());
      exit(EXIT_FAILURE);
   }

   datastream = SDL_CreateSemaphore(1);
   DataReady = 0;

   spec.freq = freq;
   if (bits==16)
   {
      spec.format = AUDIO_S16SYS;
   }
   else
   {
      spec.format = AUDIO_U8;
   }
   spec.channels = channels;
   spec.samples = SDL_BUFFER_SIZE;
   spec.callback = SDLPlayer::callback;
   spec.userdata = this;

   if (SDL_OpenAudio(&spec, NULL) < 0)
   {
      message(MSG_ERROR, "unable to open audio -- %s", SDL_GetError());
      exit(EXIT_FAILURE);
   }

   SDL_PauseAudio(0);
}

SDLPlayer::~SDLPlayer()
{
  if (!SDL_WasInit(SDL_INIT_AUDIO)) return;

  message(MSG_DEBUG, "deinit!");

  SDL_CloseAudio();
  SDL_DestroySemaphore(datastream);
  SDL_Quit();
}

void SDLPlayer::callback(void *userdata, Uint8 *stream, int len)
{
   SDLPlayer *self = (SDLPlayer *)userdata;

   if (self->DataReady==1)
   {
      memcpy(stream, self->playbuf, len);
      self->played += len;
      self->playbuf += len;
      if (self->playsize <= self->played)
      {
         self->DataReady = 0;
         SDL_SemPost(self->datastream);
      }
   }
}

void SDLPlayer::output(const void *buf, unsigned long size)
{
   SDL_SemWait(datastream);

   played = 0;
   playbuf = (unsigned char*)buf;
   playsize = size;
   DataReady = 1;
}
