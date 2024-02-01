/*
 * AdPlay/UNIX - OPL2 audio player
 * Copyright (C) 2001 - 2017 Simon Peter <dn.tlp@gmx.net>
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
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <adplug/adplug.h>
#include <adplug/emuopl.h>
#include <adplug/kemuopl.h>
#include <adplug/wemuopl.h>
#include <adplug/diskopl.h>

/*
 * Sun systems declare getopt in unistd.h,
 * other systems (Linux, Apple) use getopt.h.
 */
#if (defined(__SVR4) && defined(__sun))
#	include <unistd.h>
#else
#	ifdef HAVE_GETOPT_H
#		include <getopt.h>
#	else
#		include "getopt.h"
// ALSA now includes the system-wide getopt under Linux, so stop this
#   define _GETOPT_POSIX_H 1
#	endif
#endif

#include "defines.h"

#ifdef HAVE_ADPLUG_NUKEDOPL
#include <adplug/nemuopl.h>
#endif
#ifdef HAVE_ADPLUG_SURROUND
#include <adplug/surroundopl.h>
#endif

#include "output.h"
#include "players.h"

/***** Defines *****/

// Default file name of AdPlug's database file
#define ADPLUGDB_FILE		"adplug.db"

// Default AdPlug user's configuration subdirectory
#define ADPLUG_CONFDIR		".adplug"

// Default path to AdPlug's system-wide database file
#ifdef ADPLUG_DATA_DIR
#  define ADPLUGDB_PATH		ADPLUG_DATA_DIR "/" ADPLUGDB_FILE
#else
#  define ADPLUGDB_PATH		ADPLUGDB_FILE
#endif

/***** Typedefs *****/

typedef enum {
	Emu_Satoh,
	Emu_Ken,
	Emu_Woody,
#ifdef HAVE_ADPLUG_NUKEDOPL
	Emu_Nuked,
#endif
	Emu_Rawout,
} EmuType;

/***** Global variables *****/

static const char	*program_name;
static Player		*player = 0;		// global player object
static CAdPlugDatabase	mydb;
static Copl		*opl = 0;

/***** Configuration (and defaults) *****/

static struct {
  int			buf_size, freq, channels, bits, harmonic, message_level;
  unsigned int		subsong, loops;
  const char		*device;
  char			*userdb;
  bool			endless, showinsts, songinfo, songmessage;
  EmuType		emutype;
  Outputs		output;
} cfg = {
  2048, 44100,
#ifdef HAVE_ADPLUG_SURROUND
  2, 16, 1,  // Default to surround if available
#else
  1, 16, 0,  // Else default to mono (until stereo w/ single OPL is fixed)
#endif
  MSG_NOTE,
  (unsigned int)-1, 1,
  NULL,
  NULL,
  true, false, false, false,
  Emu_Woody,
  DEFAULT_DRIVER
};

/***** Global functions *****/

void message(int level, const char *fmt, ...)
{
  va_list argptr;

  if(cfg.message_level < level) return;

  fprintf(stderr, "%s: ", program_name);
  va_start(argptr, fmt);
  vfprintf(stderr, fmt, argptr);
  va_end(argptr);
  fprintf(stderr, "\n");
}

/***** Local functions *****/

static void usage()
/* Print usage information. */
{
  printf("Usage: %s [OPTION]... FILE...\n\n"
	 "Output selection:\n"
	 "  -e, --emulator=EMULATOR    specify emulator to use\n"
	 "  -O, --output=OUTPUT        specify output mechanism\n\n"
#ifdef DRIVER_OSS
	 "OSS driver (oss) specific:\n"
	 "  -d, --device=FILE          set sound device file to FILE\n"
	 "  -b, --buffer=SIZE          set output buffer size to SIZE\n\n"
#endif
#ifdef DRIVER_DISK
	 "Disk writer (disk) specific:\n"
	 "  -d, --device=FILE          output to FILE ('-' is stdout)\n\n"
#endif
#ifdef DRIVER_ESOUND
	 "EsounD driver (esound) specific:\n"
	 "  -d, --device=URL           URL to EsounD server host (hostname:port)\n\n"
#endif
#ifdef DRIVER_SDL
	 "SDL driver (sdl) specific:\n"
	 "  -b, --buffer=SIZE          set output buffer size to SIZE\n\n"
#endif
#ifdef DRIVER_ALSA
	 "ALSA driver (alsa) specific:\n"
	 "  -d, --device=DEVICE        set sound device to DEVICE\n"
	 "  -b, --buffer=SIZE          set output buffer size to SIZE\n\n"
#endif
	 "Playback quality:\n"
	 "  -8, --8bit                 8-bit sample quality\n"
	 "      --16bit                16-bit sample quality\n"
	 "  -f, --freq=FREQ            set sample frequency to FREQ\n"
 	 "      --surround             stereo/surround stream\n"
	 "      --stereo               stereo stream\n"
	 "      --mono                 mono stream\n\n"
	 "Informative output:\n"
	 "  -i, --instruments          display instrument names\n"
	 "  -r, --realtime             display realtime song info\n"
	 "  -m, --message              display song message\n\n"
	 "Playback:\n"
	 "  -s, --subsong=N            play subsong number N\n"
	 "  -o, --once                 play only once, don't loop\n"
	 "  -l, --loop=N               loop exactly N times\n\n"
	 "Generic:\n"
	 "  -D, --database=FILE        additionally use database file FILE\n"
	 "  -q, --quiet                be more quiet\n"
	 "  -v, --verbose              be more verbose\n"
	 "  -h, --help                 display this help and exit\n"
	 "  -V, --version              output version information and exit\n\n",
	 program_name);

  // Print list of available output mechanisms
  printf("Available emulators: satoh ken woody");
#ifdef HAVE_ADPLUG_NUKEDOPL
  printf(" nuked");
#endif
  printf(" rawout\n");
  printf("Available output mechanisms: "
#ifdef DRIVER_OSS
	 "oss "
#endif
#ifdef DRIVER_NULL
	 "null "
#endif
#ifdef DRIVER_DISK
	 "disk "
#endif
#ifdef DRIVER_ESOUND
	 "esound "
#endif
#ifdef DRIVER_QSA
	 "qsa "
#endif
#ifdef DRIVER_SDL
	 "sdl "
#endif
#ifdef DRIVER_AO
	 "ao "
#endif
#ifdef DRIVER_ALSA
	 "alsa "
#endif
	 "\n");
}

static int decode_switches(int argc, char **argv)
/*
 * Set all the option flags according to the switches specified.
 * Return the index of the first non-option argument.
 */
{
  int c;
  struct option const long_options[] = {
    {"8bit", no_argument, NULL, '8'},		// 8-bit replay
    {"16bit", no_argument, NULL, '1'},		// 16-bit replay
    {"freq", required_argument, NULL, 'f'},	// set frequency
    {"surround", no_argument, NULL, '4'},		// stereo/harmonic replay
    {"stereo", no_argument, NULL, '3'},		// stereo replay
    {"mono", no_argument, NULL, '2'},		// mono replay
    {"buffer", required_argument, NULL, 'b'},	// buffer size
    {"device", required_argument, NULL, 'd'},	// device file
    {"instruments", no_argument, NULL, 'i'},	// show instruments
    {"realtime", no_argument, NULL, 'r'},	// realtime song info
    {"message", no_argument, NULL, 'm'},	// song message
    {"subsong", no_argument, NULL, 's'},	// play subsong
    {"once", no_argument, NULL, 'o'},		// don't loop
    {"loop", required_argument, NULL, 'l'},	// loop count
    {"help", no_argument, NULL, 'h'},		// display help
    {"version", no_argument, NULL, 'V'},	// version information
    {"emulator", required_argument, NULL, 'e'},	// emulator to use
    {"output", required_argument, NULL, 'O'},	// output mechanism
    {"database", required_argument, NULL, 'D'},	// different database
    {"quiet", no_argument, NULL, 'q'},		// be more quiet
    {"verbose", no_argument, NULL, 'v'},	// be more verbose
    {NULL, 0, NULL, 0}				// end of options
  };

  while ((c = getopt_long(argc, argv, "8f:b:d:irms:ol:hVe:O:D:qv",
			  long_options, (int *)0)) != EOF) {
      switch (c) {
      case '8': cfg.bits = 8; break;
      case '1': cfg.bits = 16; break;
      case 'f': cfg.freq = atoi(optarg); break;
      case '4': cfg.channels = 2; cfg.harmonic = 1; break;
      case '3': cfg.channels = 2; cfg.harmonic = 0; break;
      case '2': cfg.channels = 1; cfg.harmonic = 0; break;
      case 'b': cfg.buf_size = atoi(optarg); break;
      case 'd': cfg.device = optarg; break;
      case 'i': cfg.showinsts = true; break;
      case 'r': cfg.songinfo = true; break;
      case 'm': cfg.songmessage = true; break;
      case 's': cfg.subsong = atoi(optarg); break;
      case 'o': cfg.endless = false; break;
      case 'l': cfg.endless = false; cfg.loops = atoi(optarg); break;
      case 'V': puts(ADPLAY_VERSION); exit(EXIT_SUCCESS);
      case 'h':	usage(); exit(EXIT_SUCCESS); break;
      case 'D':
	if(!mydb.load(optarg))
	  message(MSG_WARN, "could not open database -- %s", optarg);
	break;
      case 'O':
#ifdef DRIVER_OSS
	if(!strcmp(optarg,"oss")) cfg.output = oss;
	else
#endif
#ifdef DRIVER_NULL
	if(!strcmp(optarg,"null")) cfg.output = null;
	else
#endif
#ifdef DRIVER_DISK
	if(!strcmp(optarg,"disk")) {
	  cfg.output = disk;
	  cfg.endless = false; // endless output is almost never desired here
	}
	else
#endif
#ifdef DRIVER_ESOUND
	if(!strcmp(optarg,"esound")) cfg.output = esound;
	else
#endif
#ifdef DRIVER_QSA
	if(!strcmp(optarg,"qsa")) cfg.output = qsa;
	else
#endif
#ifdef DRIVER_ALSA
	if(!strcmp(optarg,"alsa")) cfg.output = alsa;
	else
#endif
#ifdef DRIVER_SDL
	if(!strcmp(optarg,"sdl")) cfg.output = sdl;
	else
#endif
#ifdef DRIVER_AO
	if(!strcmp(optarg,"ao")) cfg.output = ao;
	else
#endif
	{
	  message(MSG_ERROR, "unknown output method -- %s", optarg);
	  exit(EXIT_FAILURE);
	}
	break;
      case 'e':
	if(!strcmp(optarg, "satoh")) cfg.emutype = Emu_Satoh;
	else if(!strcmp(optarg, "ken")) cfg.emutype = Emu_Ken;
	else if(!strcmp(optarg, "woody")) cfg.emutype = Emu_Woody;
#ifdef HAVE_ADPLUG_NUKEDOPL
	else if(!strcmp(optarg, "nuked")) cfg.emutype = Emu_Nuked;
#endif
	else if(!strcmp(optarg, "rawout")) {
	  cfg.emutype = Emu_Rawout;
	  cfg.output = null;
	  cfg.endless = false;
	}

	else {
	  message(MSG_ERROR, "unknown emulator -- %s", optarg);
	  exit(EXIT_FAILURE);
	}
      case 'q': if(cfg.message_level) cfg.message_level--; break;
      case 'v': cfg.message_level++; break;
      }
  }
  if (!cfg.loops) cfg.loops = 1;

  return optind;
}

static void play(const char *fn, Player *pl, int subsong = -1)
/*
 * Start playback of subsong 'subsong' of file 'fn', using player
 * 'player'. If 'subsong' is not given or -1, start playback of
 * default subsong of file.
 */
{
  unsigned long i;
  unsigned long s = 0;
  unsigned long ls = 0;
  unsigned int loops = 0;

  // initialize output & player
  pl->get_opl()->init();
  delete pl->p;
  pl->p = CAdPlug::factory(fn,pl->get_opl());

  if(!pl->p) {
    message(MSG_WARN, "unknown filetype -- %s", fn);
    return;
  }

  if(subsong != -1)
    pl->p->rewind(subsong);
#ifdef HAVE_ADPLUG_GETSUBSONG
  else
    subsong = pl->p->getsubsong();
#endif

  fprintf(stderr, "Playing '%s'...\n"
	  "Type  : %s\n"
	  "Title : %s\n"
	  "Author: %s\n\n", fn, pl->p->gettype().c_str(),
	  pl->p->gettitle().c_str(), pl->p->getauthor().c_str());

  if(cfg.showinsts) {		// display instruments
    fprintf(stderr, "Instrument names:\n");
    for(i = 0;i < pl->p->getinstruments(); i++)
      fprintf(stderr, "%2lu: %s\n", i, pl->p->getinstrument(i).c_str());
    fprintf(stderr, "\n");
  }

  if(cfg.songmessage)	// display song message
    fprintf(stderr, "Song message:\n%s\n\n", pl->p->getdesc().c_str());

  // play loop
  do {
    if(cfg.songinfo)	// display song info
      fprintf(stderr, "Subsong: %d/%d, Order: %d/%d, Pattern: %d/%d, Row: %d, "
	      "Speed: %d, Timer: %.2fHz     \r",
	      subsong, pl->p->getsubsongs()-1, pl->p->getorder(),
	      pl->p->getorders(), pl->p->getpattern(), pl->p->getpatterns(),
	      pl->p->getrow(), pl->p->getspeed(), pl->p->getrefresh());

    pl->frame();
    ++s;

    if (!pl->playing) {
      if (!ls) ls = s;
      if (s == ls) {
        ++loops;
        s = 0;
      }
    }
  } while(cfg.endless || loops < cfg.loops);
}

static void shutdown(void)
/* General deinitialization handler. */
{
  if(player) delete player;
  if(opl) delete opl;
  // Try to properly reposition terminal cursor, if Ctrl+C is used to exit.
  printf("\n\n");
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

/***** Main program *****/

int main(int argc, char **argv)
{
  int			optind, i;
  const char		*homedir;
  char			*userdb = NULL;

  // init
  program_name = argv[0];
  atexit(shutdown);
  signal(SIGINT, sighandler); signal(SIGTERM, sighandler);

  // Try user's home directory first, before trying the default location.
  homedir = getenv("HOME");
  if(homedir) {
    userdb = (char *)malloc(strlen(homedir) + strlen(ADPLUG_CONFDIR) +
			    strlen(ADPLUGDB_FILE) + 3);
    strcpy(userdb, homedir); strcat(userdb, "/" ADPLUG_CONFDIR "/");
    strcat(userdb, ADPLUGDB_FILE);
  }

  // parse commandline
  optind = decode_switches(argc,argv);
  if(optind == argc) {	// no filename given
    fprintf(stderr, "%s: need at least one file for playback\n", program_name);
    fprintf(stderr, "Try '%s --help' for more information.\n", program_name);
    if(userdb) free(userdb);
    exit(EXIT_FAILURE);
  }
  if(argc - optind > 1) cfg.endless = false;	// more than 1 file given

  // init emulator
  switch(cfg.emutype) {
  case Emu_Satoh:
  	if (cfg.harmonic) {
#ifdef HAVE_ADPLUG_SURROUND
      COPLprops a, b;
      a.use16bit = b.use16bit = cfg.bits == 16;
      a.stereo = b.stereo = false;
      a.opl = new CEmuopl(cfg.freq, a.use16bit, a.stereo);
      b.opl = new CEmuopl(cfg.freq, b.use16bit, b.stereo);
      opl = new CSurroundopl(&a, &b, cfg.bits == 16);
      // CSurroundopl now owns a.opl and b.opl and will free upon destruction
#else
      fprintf(stderr, "Surround requires AdPlug v2.2 or newer.  Use --mono "
      	"or upgrade and recompile AdPlay.\n");
      if(userdb) free(userdb);
      exit(EXIT_FAILURE);
#endif
  	} else {
      opl = new CEmuopl(cfg.freq, cfg.bits == 16, cfg.channels == 2);
  	}
    break;
  case Emu_Ken:
  	if (cfg.harmonic) {
#ifdef HAVE_ADPLUG_SURROUND
#ifndef CKEMUOPL_MULTIINSTANCE
	  fprintf(stderr, "%s: Sorry, Ken's emulator only supports one instance "
	    "so does not work properly in surround mode in old versions of "
	    "the adplug library.\n", program_name);
#endif
      COPLprops a, b;
      a.use16bit = b.use16bit = cfg.bits == 16;
      a.stereo = b.stereo = false;
      a.opl = new CKemuopl(cfg.freq, a.use16bit, a.stereo);
      b.opl = new CKemuopl(cfg.freq, b.use16bit, b.stereo);
      opl = new CSurroundopl(&a, &b, cfg.bits == 16);
      // CSurroundopl now owns a and b and will free upon destruction
#else
      fprintf(stderr, "Surround requires AdPlug v2.2 or newer.  Use --mono "
      	"or upgrade and recompile AdPlay.\n");
      if(userdb) free(userdb);
      exit(EXIT_FAILURE);
#endif
  	} else {
  		opl = new CKemuopl(cfg.freq, cfg.bits == 16, cfg.channels == 2);
  	}
    break;
   case Emu_Woody:
  	if (cfg.harmonic) {
#ifdef HAVE_ADPLUG_SURROUND
      COPLprops a, b;
      a.use16bit = b.use16bit = cfg.bits == 16;
      a.stereo = b.stereo = false;
      a.opl = new CWemuopl(cfg.freq, a.use16bit, a.stereo);
      b.opl = new CWemuopl(cfg.freq, b.use16bit, b.stereo);
      opl = new CSurroundopl(&a, &b, cfg.bits == 16);
      // CSurroundopl now owns a and b and will free upon destruction
#else
      fprintf(stderr, "Surround requires AdPlug v2.2 or newer.  Use --mono "
      	"or upgrade and recompile AdPlay.\n");
      if(userdb) free(userdb);
      exit(EXIT_FAILURE);
#endif
  	} else {
      opl = new CWemuopl(cfg.freq, cfg.bits == 16, cfg.channels == 2);
  	}
    break;
#ifdef HAVE_ADPLUG_NUKEDOPL
  case Emu_Nuked:
    if (cfg.harmonic) {
      COPLprops a, b;
      a.use16bit = b.use16bit = true; // Nuked only supports 16-bit
      a.stereo = b.stereo = true; // Nuked only supports stereo
      a.opl = new CNemuopl(cfg.freq);
      b.opl = new CNemuopl(cfg.freq);
      opl = new CSurroundopl(&a, &b, cfg.bits == 16); // SurroundOPL can convert to 8-bit though
      // CSurroundopl now owns a and b and will free upon destruction
  	} else {
  		if(cfg.bits != 16 || cfg.channels != 2) {
  			fprintf(stderr, "Sorry, Nuked OPL3 emulator only works in stereo 16 bits. "
  				"Use --stereo and --16bit options.\n");
  			if(userdb) free(userdb);
  			exit(EXIT_FAILURE);
  		} else {
  			opl = new CNemuopl(cfg.freq);
  		}
  	}
  	break;
#endif
  case Emu_Rawout:
    opl = new CDiskopl(cfg.device);
  }

  // init player
  switch(cfg.output) {
  case none:
    message(MSG_PANIC, "no output methods compiled in");
    exit(EXIT_FAILURE);
#ifdef DRIVER_OSS
  case oss:
    player = new OSSPlayer(opl, cfg.device, cfg.bits, cfg.channels, cfg.freq,
			   cfg.buf_size);
    break;
#endif
#ifdef DRIVER_NULL
  case null:
    player = new NullOutput(opl);
    break;
#endif
#ifdef DRIVER_DISK
  case disk:
    player = new DiskWriter(opl, cfg.device, cfg.bits, cfg.channels, cfg.freq);
    break;
#endif
#ifdef DRIVER_ESOUND
  case esound:
    player = new EsoundPlayer(opl, cfg.bits, cfg.channels, cfg.freq,
			      cfg.device);
    break;
#endif
#ifdef DRIVER_QSA
  case qsa:
    player = new QSAPlayer(opl, cfg.bits, cfg.channels, cfg.freq);
    break;
#endif
#ifdef DRIVER_AO
  case ao:
    player = new AOPlayer(opl, cfg.device, cfg.bits, cfg.channels, cfg.freq, cfg.buf_size);
    break;
#endif
#ifdef DRIVER_SDL
  case sdl:
    player = new SDLPlayer(opl, cfg.bits, cfg.channels, cfg.freq, cfg.buf_size);
    break;
#endif
#ifdef DRIVER_ALSA
  case alsa:
    player = new ALSAPlayer(opl, cfg.device, cfg.bits, cfg.channels, cfg.freq,
			    cfg.buf_size);
    break;
#endif
  default:
    message(MSG_ERROR, "output method not available");
    exit(EXIT_FAILURE);
  }

  // load database
  if(userdb) { mydb.load(userdb); free(userdb); }
  mydb.load(ADPLUGDB_PATH);
  CAdPlug::set_database(&mydb);

  // play all files from commandline
  for(i=optind;i<argc;i++)
    play(argv[i],player,cfg.subsong);

  // deinit
  exit(EXIT_SUCCESS);
}
