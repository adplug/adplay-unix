/*
 * AdPlay/UNIX - OPL2 audio player
 * Copyright (C) 2001 - 2003 Simon Peter <dn.tlp@gmx.net>
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

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <binwrap.h>

#include "defines.h"
#include "disk.h"

DiskWriter::DiskWriter(const char *filename, unsigned char nbits, unsigned char nchannels,
		       unsigned long nfreq)
  : EmuPlayer(nbits,nchannels,nfreq), f(0), samplesize(0)
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
    fprintf(stderr, "%s: No output filename specified!\n", program_name);
    exit(EXIT_FAILURE);
  }

  // If filename is '-', output to stdout
  if(strcmp(filename, "-")) {
    out.open(filename, ios::out | ios::bin);
    if(!out.is_open()) {
      fprintf(stderr, "%s: Cannot open file for output -- %s\n", program_name,
	      filename);
      exit(EXIT_FAILURE);
    }
    f = new binowstream(&out);
  } else {
    f = new binowstream(&cout);
  }

  f->setFlag(binio::BigEndian, false);

  // Write Microsoft RIFF WAVE header
  f->writeInt(riff_header.chunkid, 4);
  f->writeInt(riff_header.chunksize, 4);
  f->writeInt(riff_header.format, 4);
  f->writeInt(riff_header.subchunk1id, 4);
  f->writeInt(riff_header.subchunk1size, 4);
  f->writeInt(riff_header.audioformat, 2);
  f->writeInt(riff_header.numchannels, 2);
  f->writeInt(riff_header.samplerate, 4);
  f->writeInt(riff_header.byterate, 4);
  f->writeInt(riff_header.blockalign, 2);
  f->writeInt(riff_header.bitspersample, 2);
  f->writeInt(riff_header.subchunk2id, 4);
  f->writeInt(riff_header.subchunk2size, 4);
}

DiskWriter::~DiskWriter()
{
  if(!f) return;

  if(samplesize % 2) { // Wave data must end on an even byte boundary
    f->writeInt(0, 1);
    samplesize++;
  }

  // Write file sizes
  f->seek(40); f->writeInt(samplesize, 4);
  samplesize += 36; // make absolute filesize (add header size)
  f->seek(4); f->writeInt(samplesize, 4);

  // end disk writing
  delete f;
  out.close();
}

void DiskWriter::output(const void *buf, unsigned long size)
{
  f->writeString((char *)buf, size);
  samplesize += size;
}
