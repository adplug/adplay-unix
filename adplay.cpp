/*
 * AdPlay/Linux - A Linux Console frontend for AdPlug.
 * Copyright (C) 2001 Simon Peter <dn.tlp@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <iostream.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/soundcard.h>
#include <getopt.h>
#include <pthread.h>
#include <adplug/adplug.h>
#include <adplug/emuopl.h>

#define VERSION	"AdPlay/Linux v1.0, (c) 2001 Simon Peter <dn.tlp@gmx.net>, et al."
#define USAGE	"[OPTION]... FILE..."

// defaults
#define BUF_SIZE	512		// buffer size in samples
#define FREQ		44100		// replay frequency
#define CHANNELS	1		// number of channels
#define BITS		16		// bit depth
#define DEVICE		"/dev/dsp"	// audio device file
#define ENDLESS		false		// play endlessly?
#define SHOWINSTS	false		// show instruments

CAdPlug	adplug;				// global AdPlug object
CPlayer *p;				// global player object
CEmuopl	*opl;				// global emulator object
float	decode_pos_ms;			// current decode position in ms
int	audio_fd;			// audio device file
char	*prgname;			// program executable name

// configuration variables
int	buf_size=BUF_SIZE,freq=FREQ,channels=CHANNELS,bits=BITS;
char	*device=DEVICE;
bool	endless=ENDLESS,showinsts=SHOWINSTS;

#ifndef min
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif

inline int getsampsize()
{ return (channels * (bits / 8)); }

void display_usage()
{
	cout << "Usage: " << prgname << " " << USAGE << endl;
}

void display_help()
{
	display_usage();
	cout << "Playback known filetypes of given FILEs." << endl << endl <<
		"  -8, --8bit		8-bit replay" << endl <<
		"  -f, --freq=FREQ	set frequency to FREQ" << endl <<
		"  -s, --stereo		stereo playback" << endl <<
		"  -b, --buffer=SIZE	set buffer size to SIZE" << endl <<
		"  -d, --device=FILE	set sound device file to FILE" << endl <<
		"  -i, --instruments	display instrument names" << endl <<
		"      --help		display this help and exit" << endl <<
		"      --version		output version information and exit" << endl;
}

void *replay_thread(void *killswitch)
{
	char	*pos,*audiobuf;
	long	i,towrite,minicnt=0;
	bool	playing=true;

	audiobuf = new char [buf_size * getsampsize()];
	while((playing || endless) && !(*(bool *)killswitch)) {
		towrite = buf_size; pos = audiobuf;
		while(towrite > 0) {
			while(minicnt < 0)
			{
				minicnt += freq;
				playing = p->update();
				decode_pos_ms += 1000/p->getrefresh();
			}
			i = min(towrite,(long)(minicnt/p->getrefresh()+4)&~3);
			opl->update((short *)pos, i);
			pos += i * 2; towrite -= i;
			minicnt -= (long)(p->getrefresh()*i);
		}
		write(audio_fd, audiobuf, buf_size*getsampsize());
	}
	delete [] audiobuf;
	return 0;
}

int parse_options(int argc, char *argv[])
{
	int		option;
	struct option	opts[]={
		{"8bit",no_argument,NULL,'8'},		// 8-bit replay
		{"freq",required_argument,NULL,'f'},	// set frequency
		{"stereo",no_argument,NULL,'s'},	// stereo replay
		{"buffer",required_argument,NULL,'b'},	// buffer size
		{"device",required_argument,NULL,'d'},	// device file
		{"instruments",no_argument,NULL,'i'},	// show instruments
		{"help",no_argument,NULL,'h'},		// display help
		{"version",no_argument,NULL,'v'},	// version information
		{0,0,0,0}				// end of options
	};

	while((option=getopt_long(argc,argv,"8f:sb:d:i",opts,NULL)) != -1)
		switch(option) {
		case '8':
			bits = 8;
			break;
		case 'f':
			freq = atoi(optarg);
			break;
		case 's':
			channels = 2;
			break;
		case 'b':
			buf_size = atoi(optarg);
			break;
		case 'd':
			device = optarg;
			break;
		case 'i':
			showinsts = true;
			break;
		case 'h':
			display_help();
			exit(0);
			break;
		case 'v':
			cout << VERSION << endl;
			exit(0);
			break;
		}

	if(optind == argc) {
		display_usage();
		cout << "Try '" << prgname << " --help' for more information." << endl;
		exit(1);
	}

	return optind;
}

void play(char *fn)
{
	pthread_t	th;
	bool		killswitch=false;
	char		inkey;
	unsigned int	i;

	if(!(p = adplug.factory(fn, opl))) {
		cout << prgname << ": " << fn << ": Unknown filetype" << endl;
		return;
	}

	pthread_create(&th,NULL,replay_thread,&killswitch);

	cout << "Playing '" << fn << "'..." << endl <<
		"Type : " << p->gettype() << endl <<
		"Title: " << p->gettitle() << endl;

	// display instruments
	if(showinsts) {
		cout << "Instrument names:" << endl;
		for(i=0;i<p->getinstruments();i++)
			cout << i << ": " << p->getinstrument(i) << endl;
		cout << endl;
	}

	do {
		inkey = getchar();
		switch(inkey) {
		case 'f':	// fast forward
			break;
		}
	} while(inkey != '\n' && inkey != 'q');

	killswitch = true;
	pthread_join(th, NULL);
	delete p;
	if(inkey == 'q') {
		delete opl;
		close(audio_fd);
		exit(0);
	}
}

int main(int argc, char *argv[])
{
	int	optind,i,format;

	// parse commandline
	prgname = argv[0];
	optind = parse_options(argc,argv);

	// open audio device
	if((audio_fd = open(device, O_WRONLY, 0)) == -1) {
		perror(device);
		exit(1);
	}
	format = (bits == 16 ? AFMT_S16_LE : AFMT_S16_LE);
	ioctl(audio_fd, SNDCTL_DSP_SETFMT, &format);
	ioctl(audio_fd, SNDCTL_DSP_CHANNELS, &channels);
	ioctl(audio_fd, SNDCTL_DSP_SPEED, &freq);

	// main loop
	opl = new CEmuopl(freq,bits == 16 ? true : false,channels == 2 ? true : false);
	for(i=optind;i<argc;i++) {
		opl->init();
		play(argv[i]);
	}

	// deinit
	delete opl;
	close(audio_fd);
	return 0;
}
