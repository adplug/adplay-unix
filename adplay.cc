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

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/soundcard.h>
#include <adplug/adplug.h>
#include <adplug/emuopl.h>

#include "getopt.h"

// defines
#define ADPLAY_VERSION	"AdPlay/UNIX " VERSION	// AdPlay/UNIX version string

// defaults
#define BUF_SIZE	512		// buffer size in samples
#define FREQ		44100		// replay frequency
#define CHANNELS	1		// number of channels
#define BITS		16		// bit depth
#define DEVICE		"/dev/dsp"	// audio device file
#define ENDLESS		true		// play endlessly
#define SHOWINSTS	false		// show instruments
#define SONGINFO	false		// show song info
#define SONGMESSAGE	false		// show song message
#define SUBSONG		0		// default subsong

// global variables
CEmuopl *opl;			// global emulator object
int audio_fd;			// audio device file
const char *program_name;	// Program executable name

// option variables
int		buf_size=BUF_SIZE,freq=FREQ,channels=CHANNELS,bits=BITS;
unsigned int	subsong=SUBSONG;
char		*device=DEVICE;
bool		endless=ENDLESS,showinsts=SHOWINSTS,songinfo=SONGINFO,songmessage=SONGMESSAGE;

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#define getsampsize (channels * (bits / 8))

static void usage (int exit_status)
/*
 * Print usage information and exit with exit_status.
 */
{
	puts(ADPLAY_VERSION " - OPL2 audio player");
	printf("Usage: %s [OPTION]... [FILE]...\n",program_name);
	printf("Options:\n\
  -8, --8bit                 8-bit replay\n\
      --16bit                16-bit replay\n\
  -f, --freq=FREQ            set frequency to FREQ\n\
      --stereo               stereo playback\n\
      --mono                 mono playback\n\
  -b, --buffer=SIZE          set buffer size to SIZE\n\
  -d, --device=FILE          set sound device file to FILE\n\
  -i, --instruments          display instrument names\n\
  -r, --realtime             display realtime song info\n\
  -m, --message              display song message\n\
  -s, --subsong=N            play subsong number N\n\
  -o, --once                 play only once, don't loop\n\
  -h, --help                 display this help and exit\n\
  -V, --version              output version information and exit\n");

	exit(exit_status);
}

static int decode_switches(int argc, char **argv)
/*
 * Set all the option flags according to the switches specified.
 * Return the index of the first non-option argument.
 */
{
  int c;
  struct option const long_options[] = {
    {"8bit",no_argument,NULL,'8'},		// 8-bit replay
    {"16bit",no_argument,NULL,'1'},		// 16-bit replay
    {"freq",required_argument,NULL,'f'},	// set frequency
    {"stereo",no_argument,NULL,'3'},		// stereo replay
    {"mono",no_argument,NULL,'2'},		// mono replay
    {"buffer",required_argument,NULL,'b'},	// buffer size
    {"device",required_argument,NULL,'d'},	// device file
    {"instruments",no_argument,NULL,'i'},	// show instruments
    {"realtime",no_argument,NULL,'r'},		// realtime song info
    {"message",no_argument,NULL,'m'},		// song message
    {"subsong",no_argument,NULL,'s'},		// play subsong
    {"once",no_argument,NULL,'o'},		// don't loop
    {"help", no_argument, 0, 'h'},		// display help
    {"version", no_argument, 0, 'V'},		// version information
    {NULL, 0, NULL, 0}				// end of options
  };

  while ((c = getopt_long(argc, argv, "8f:b:d:irms:ohV", long_options, (int *) 0)) != EOF) {
      switch (c) {
      case '8': bits = 8; break;
      case '1': bits = 16; break;
      case 'f': freq = atoi(optarg); break;
      case '3': channels = 2; break;
      case '2': channels = 1; break;
      case 'b': buf_size = atoi(optarg); break;
      case 'd': device = optarg; break;
      case 'i': showinsts = true; break;
      case 'r': songinfo = true; break;
      case 'm': songmessage = true; break;
      case 's': subsong = atoi(optarg); break;
      case 'o': endless = false; break;
      case 'V': puts(ADPLAY_VERSION); exit(0);
      case 'h':	usage(0);
      default: usage(1);
      }
  }

  return optind;
}

void play(char *fn)
{
	CPlayer *p = CAdPlug::factory(fn,opl);
	char *pos,*audiobuf;
	long i,towrite,minicnt=0;
	bool playing=true;

	if(!p) {
		cout << program_name << ": " << fn << ": Unknown filetype" << endl;
		return;
	}

	p->rewind(subsong);

	cout << "Playing '" << fn << "'..." << endl <<
		"Type  : " << p->gettype() << endl <<
		"Title : " << p->gettitle() << endl <<
		"Author: " << p->getauthor() << endl << endl;

	if(showinsts) {		// display instruments
		cout << "Instrument names:" << endl;
		for(i=0;i<(long)p->getinstruments();i++)
			cout << i << ": " << p->getinstrument(i) << endl;
		cout << endl;
	}

	if(songmessage) {	// display song message
		cout << "Song message:" << endl;
		cout << p->getdesc() << endl << endl;
	}

	// play loop
	audiobuf = new char [buf_size * getsampsize];
	while(playing || endless) {
		if(songinfo) {	// display song info
			printf("Subsong: %d/%d, Order: %d/%d, Pattern: %d/%d, Row: %d, Speed: %d, Timer: %.2fHz     \r",
				subsong,p->getsubsongs()-1,p->getorder(),p->getorders(),p->getpattern(),
				p->getpatterns(),p->getrow(),p->getspeed(),p->getrefresh());
		}

		towrite = buf_size; pos = audiobuf;
		while(towrite > 0) {
			while(minicnt < 0)
			{
				minicnt += freq;
				playing = p->update();
			}
			i = min(towrite,(long)(minicnt/p->getrefresh()+4)&~3);
			opl->update((short *)pos, i);
			pos += i * getsampsize; towrite -= i;
			minicnt -= (long)(p->getrefresh()*i);
		}
		write(audio_fd, audiobuf, buf_size*getsampsize);
	}

	delete [] audiobuf;
	delete p;
}

int main(int argc, char **argv)
{
  int	optind,i,format;

  // parse commandline
  program_name = argv[0];
  optind = decode_switches(argc,argv);
  if(optind == argc) usage(1);			// no filename given
  if(argc - optind > 1) endless = false;	// more than 1 file given

  // open OSS audio device
  if((audio_fd = open(device, O_WRONLY, 0)) == -1) {
    perror(device);
    exit(EXIT_FAILURE);
  }
  format = (bits == 16 ? AFMT_S16_LE : AFMT_S8);
  ioctl(audio_fd, SNDCTL_DSP_SETFMT, &format);
  ioctl(audio_fd, SOUND_PCM_WRITE_CHANNELS, &channels);
  ioctl(audio_fd, SNDCTL_DSP_SPEED, &freq);

  // main loop
  opl = new CEmuopl(freq,bits == 16,channels == 2);
  for(i=optind;i<argc;i++) {	// play all files from commandline
    opl->init();
    play(argv[i]);
  }

  // deinit
  delete opl;
  close(audio_fd);
  exit(0);
}
