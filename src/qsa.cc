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

#include "qsa.h"

#include <errno.h>

QSAPlayer::QSAPlayer(unsigned char bits, int channels, int freq)
  : EmuPlayer(bits, channels, freq, QSA_FRAG_SIZE)
{
   int rval;
   snd_pcm_channel_params_t cparams;
   audio_handle=NULL;

   rval = snd_pcm_open_preferred(&audio_handle, NULL, NULL, SND_PCM_OPEN_PLAYBACK);
   if (rval < 0)
   {
      exit(EXIT_FAILURE);
   }

   memset(&cparams, 0, sizeof(snd_pcm_channel_params_t));

   cparams.channel = SND_PCM_CHANNEL_PLAYBACK;
   cparams.mode = SND_PCM_MODE_BLOCK;
   cparams.start_mode = SND_PCM_START_FULL;
   cparams.stop_mode  = SND_PCM_STOP_STOP;
   if (bits==16)
   {
      cparams.format.format = SND_PCM_SFMT_S16_LE;
   }
   else
   {
      cparams.format.format = SND_PCM_SFMT_U8;
   }
   cparams.format.interleave = 1;
   cparams.format.rate = freq;
   cparams.format.voices = channels;
   cparams.buf.block.frag_size = QSA_FRAG_SIZE;
   cparams.buf.block.frags_min = 4;
   cparams.buf.block.frags_max = 8;

   rval = snd_pcm_plugin_params(audio_handle, &cparams);
   if (rval < 0)
   {
      exit(EXIT_FAILURE);
   }

   rval = snd_pcm_plugin_prepare(audio_handle, SND_PCM_CHANNEL_PLAYBACK);
   if (rval < 0)
   {
      exit(EXIT_FAILURE);
   }
}

QSAPlayer::~QSAPlayer()
{
   int rval;

   if (audio_handle != NULL)
   {
      rval = snd_pcm_plugin_flush(audio_handle, SND_PCM_CHANNEL_PLAYBACK);
      rval = snd_pcm_close(audio_handle);
      audio_handle = NULL;
   }
}

void QSAPlayer::output(const void *buf, unsigned long size)
{
   int rval;
   unsigned long written;
   unsigned long towrite;
   snd_pcm_channel_status_t cstatus;
   
   towrite=size;

   do {
      written = snd_pcm_plugin_write(audio_handle, buf, towrite);

      if (written != towrite)
      {
         if((errno == EINVAL) || (errno == EIO))
         {
            memset(&cstatus, 0, sizeof(cstatus));
            if (snd_pcm_plugin_status(audio_handle, &cstatus) < 0 )
            {
               return;
            }	
            if ((cstatus.status == SND_PCM_STATUS_UNDERRUN) ||
                (cstatus.status == SND_PCM_STATUS_READY))
            {
               if (snd_pcm_plugin_prepare (audio_handle, SND_PCM_CHANNEL_PLAYBACK) < 0 )
               {
                  return;
               }
            }		        		
            continue;
         }
      }
      else
      {
         break;
      }
   } while (1);
}
