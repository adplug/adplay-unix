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
#include <string.h>
#include <signal.h>
#include <adplug/adplug.h>
#include <adplug/emuopl.h>

#include "defines.h"
#include "getopt.h"
#include "output.h"
#include "players.h"

/***** global variables *****/
const char *program_name;
static Player *player = 0;		// global player object

/***** configuration (and defaults) *****/
static struct {
  int buf_size, freq, channels, bits;
  unsigned int subsong;
  char *device;
  bool endless, showinsts, songinfo, songmessage;
  Outputs output;
} cfg = { 512, 44100, 1, 16, 0, 0, true, false, false, false, DEFAULT_DRIVER };

/***** local functions *****/

static void usage(int exit_status)
/* Print usage information and exit with exit_status. */
{
  cout << "Usage: " << program_name << " [OPTION]... FILE..." << endl;
  cout << "\
Output selection:\n\
  -O, --output=OUTPUT        Specify output mechanism.\n\
\n\
OSS driver (oss) specific:\n\
  -d, --device=FILE          set sound device file to FILE\n\
  -b, --buffer=SIZE          set output buffer size to SIZE\n\
\n\
Disk writer (disk) specific:\n\
  -d, --device=FILE          output to FILE\n\
\n\
EsounD driver (esound) specific:\n\
  -d, --device=URL           URL to EsounD server host (hostname:port)\n\
\n\
Playback quality:\n\
  -8, --8bit                 8-bit sample quality\n\
      --16bit                16-bit sample quality\n\
  -f, --freq=FREQ            set sample frequency to FREQ\n\
      --stereo               stereo stream\n\
      --mono                 mono stream\n\
\n\
Informative output:\n\
  -i, --instruments          display instrument names\n\
  -r, --realtime             display realtime song info\n\
  -m, --message              display song message\n\
\n\
Playback:
  -s, --subsong=N            play subsong number N\n\
  -o, --once                 play only once, don't loop\n\
\n\
Generic:\n\
  -h, --help                 display this help and exit\n\
  -V, --version              output version information and exit" << endl << endl;

  // Build list of available output mechanisms
  cout << "Available output mechanisms: ";
#ifdef DRIVER_OSS
  cout << "oss ";
#endif
#ifdef DRIVER_NULL
  cout << "null ";
#endif
#ifdef DRIVER_DISK
  cout << "disk ";
#endif
#ifdef DRIVER_ESOUND
  cout << "esound ";
#endif
  cout << endl;

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
    {"output",required_argument,NULL,'O'},	// output mechanism
    {NULL, 0, NULL, 0}				// end of options
  };

  while ((c = getopt_long(argc, argv, "8f:b:d:irms:ohVO:", long_options, (int *) 0)) != EOF) {
      switch (c) {
      case '8': cfg.bits = 8; break;
      case '1': cfg.bits = 16; break;
      case 'f': cfg.freq = atoi(optarg); break;
      case '3': cfg.channels = 2; break;
      case '2': cfg.channels = 1; break;
      case 'b': cfg.buf_size = atoi(optarg); break;
      case 'd': cfg.device = optarg; break;
      case 'i': cfg.showinsts = true; break;
      case 'r': cfg.songinfo = true; break;
      case 'm': cfg.songmessage = true; break;
      case 's': cfg.subsong = atoi(optarg); break;
      case 'o': cfg.endless = false; break;
      case 'V': puts(ADPLAY_VERSION); exit(EXIT_SUCCESS);
      case 'h':	usage(0); break;
      case 'O':
	if(!strcmp(optarg,"oss")) cfg.output = oss; else
	  if(!strcmp(optarg,"null")) cfg.output = null; else
	    if(!strcmp(optarg,"disk")) {
	      cfg.output = disk;
	      cfg.endless = false; // endless output is almost never desired here...
	    } else
	      if(!strcmp(optarg,"esound")) cfg.output = esound; else {
		cout << program_name << ": unknown output method -- " << optarg << endl;
		exit(EXIT_FAILURE);
	      }
	break;
      }
  }

  return optind;
}

static void play(const char *fn, Player *pl, unsigned int subsong)
/* Start playback of subsong 'subsong' of file 'fn', using player 'player'. */
{
  // initialize output & player
  pl->get_opl()->init();
  pl->p = CAdPlug::factory(fn,pl->get_opl());

  if(!pl->p) {
    cout << program_name << ": unknown filetype -- " << fn << endl;
    return;
  }

  pl->p->rewind(subsong);

  cout << "Playing '" << fn << "'..." << endl <<
    "Type  : " << pl->p->gettype() << endl <<
    "Title : " << pl->p->gettitle() << endl <<
    "Author: " << pl->p->getauthor() << endl << endl;

  if(cfg.showinsts) {		// display instruments
    cout << "Instrument names:" << endl;
    for(unsigned long i=0;i<pl->p->getinstruments();i++)
      cout << i << ": " << pl->p->getinstrument(i) << endl;
    cout << endl;
  }

  if(cfg.songmessage) {	// display song message
    cout << "Song message:" << endl;
    cout << pl->p->getdesc() << endl << endl;
  }

  // play loop
  do {
    if(cfg.songinfo)	// display song info
      printf("Subsong: %d/%d, Order: %d/%d, Pattern: %d/%d, Row: %d, Speed: %d, Timer: %.2fHz     \r",
	     subsong,pl->p->getsubsongs()-1,pl->p->getorder(),pl->p->getorders(),
	     pl->p->getpattern(),pl->p->getpatterns(),pl->p->getrow(),
	     pl->p->getspeed(),pl->p->getrefresh());

    pl->frame();
  } while(pl->playing || cfg.endless);
}

static void shutdown(void)
/* General deinitialization handler. */
{
  if(player) delete player;
}

static void sighandler(int signal)
/* Handles all kinds of signals. */
{
  switch(signal) {
  case SIGINT:
  case SIGTERM:
    exit(EXIT_SUCCESS);
  }
}

/***** main program *****/

int main(int argc, char **argv)
{
  int optind,i;

  // init
  program_name = argv[0];
  atexit(shutdown);
  signal(SIGINT, sighandler); signal(SIGTERM, sighandler);

  // parse commandline
  optind = decode_switches(argc,argv);
  if(optind == argc) {	// no filename given
    cout << program_name << ": need at least one file for playback!" << endl;
    usage(1);
  }
  if(argc - optind > 1) cfg.endless = false;	// more than 1 file given

  // init player
  switch(cfg.output) {
  case none:
    cout << program_name <<": you got NO output mechanisms compiled in! You must be crazy..." << endl;
    exit(EXIT_FAILURE);
#ifdef DRIVER_OSS
  case oss: player = new OSSPlayer(cfg.device, cfg.bits, cfg.channels, cfg.freq, cfg.buf_size); break;
#endif
#ifdef DRIVER_NULL
  case null: player = new NullOutput(); break;
#endif
#ifdef DRIVER_DISK
  case disk: player = new DiskWriter(cfg.device, cfg.bits, cfg.channels, cfg.freq); break;
#endif
#ifdef DRIVER_ESOUND
  case esound: player = new EsoundPlayer(cfg.bits, cfg.channels, cfg.freq, cfg.device); break;
#endif
  default:
    cout << program_name << ": output method not available!" << endl;
    exit(EXIT_FAILURE);
  }

  // play all files from commandline
  for(i=optind;i<argc;i++)
    play(argv[i],player,cfg.subsong);

  // deinit
  exit(EXIT_SUCCESS);
}
