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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "defines.h"
#include "disk.h"

DiskWriter::DiskWriter(const char *filename, unsigned char nbits, unsigned char nchannels,
		       unsigned long nfreq)
  : EmuPlayer(nbits,nchannels,nfreq), samplesize(0)
{
  const struct {
    unsigned long chunkid, chunksize, format, subchunk1id, subchunk1size;
    unsigned short audioformat, numchannels;
    unsigned long samplerate, byterate;
    unsigned short blockalign, bitspersample;
    unsigned long subchunk2id, subchunk2size;
  } riff_header = { 0x46464952l, 36l, 0x45564157l, 0x20746d66l, 16l,
		    1, nchannels,
		    nfreq, nfreq * getsampsize(),
		    getsampsize(), nbits,
		    0x61746164l, 0l };

  if(!filename) {
    printf("%s: No output filename specified!\n", program_name);
    exit(EXIT_FAILURE);
  }

  // If filename is '-', output to stdout
  if(strcmp(filename, "-")) {
    if(!(f = fopen(filename,"wb"))) {
      perror(filename);
      exit(EXIT_FAILURE);
    }
  } else
    f = stdout;

  // Write Microsoft RIFF WAVE header
  fwrite(&riff_header,sizeof(riff_header),1,f);
}

DiskWriter::~DiskWriter()
{
  if(samplesize % 2) { // Wave data must end on an even byte boundary
    fputc(0, f);
    samplesize++;
  }

  // Write file sizes
  fseek(f, 40, SEEK_SET); fwrite(&samplesize, 4, 1, f);
  samplesize += 36; // make absolute filesize (add header size)
  fseek(f, 4, SEEK_SET); fwrite(&samplesize, 4, 1, f);

  fclose(f); // end disk writing
}

void DiskWriter::output(const void *buf, unsigned long size)
{
  fwrite(buf,size,1,f);
  samplesize += size;
}
