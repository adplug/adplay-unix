/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999, 2000, 2001 Simon Peter, <dn.tlp@gmx.net>, et al.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 * adplay.cpp - AdPlay/Linux, by Simon Peter (dn.tlp@gmx.net)
 */

#include <iostream.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/soundcard.h>

#include "adplug.h"
#include "opl.h"
#include "emuopl.h"

#define BUF_SIZE	512		// buffer size in samples

#define FREQ		44100
#define CHANNELS	1

int	audio_fd;
char	*pos,audio_buffer[BUF_SIZE*2];

long min(long a, long b)
{
	if(a < b)
		return a;
	else
		return b;
}

int main(int argc, char *argv[])
{
	CAdPlug		adplug;
	CEmuopl		opl(44100,true,false);
	CPlayer		*p;

	long	i,towrite,minicnt=0;
	bool	playing=true;
	float	decode_pos_ms=0.0f;
	int	format;

	if(argc < 2) {	// filename is a must
		cout << "syntax: " << argv[0] << " <filename>" << endl;
		exit(1);
	}

	if(!(p = adplug.factory(argv[1], &opl))) {
		cout << "wrong filetype!" << endl;
		delete p;
		exit(2);
	}

	cout << "Type : " << p->gettype() << endl <<
		"Title: " << p->gettitle() << endl;

	// open audio device
	if((audio_fd = open("/dev/dsp", O_WRONLY, 0)) == -1) {
		perror("/dev/dsp");
		delete p;
		exit(1);
	}

	format = AFMT_S16_LE;
	ioctl(audio_fd, SNDCTL_DSP_SETFMT, &format);
	format = CHANNELS;
	ioctl(audio_fd, SNDCTL_DSP_CHANNELS, &format);
	format = FREQ;
	ioctl(audio_fd, SNDCTL_DSP_SPEED, &format);

	fflush(NULL);
	setvbuf(stdin,NULL,_IONBF,0);
	fcntl(STDIN_FILENO,F_SETFL,O_NONBLOCK);

	// do some output
	while(playing) {
		towrite = BUF_SIZE; pos = audio_buffer;
		while(towrite > 0) {
			while(minicnt < 0)
			{
				minicnt += FREQ;
				playing = p->update();
				decode_pos_ms += 1000/p->getrefresh();
			}
			i = min(towrite,(long)(minicnt/p->getrefresh()+4)&~3);
			opl.update((short *)pos, i);
			pos += i * 2; towrite -= i;
			minicnt -= (long)(p->getrefresh()*i);
		}
		write(audio_fd, audio_buffer, BUF_SIZE*2);
		printf("Playing... (%d.%2d seconds)    \r",(int)(decode_pos_ms/1000),(int)(decode_pos_ms/10)%100);
		fflush(NULL);
		if(getchar() == 'q')
			break;
	}

	close(audio_fd);
	cout << endl;
	return 0;
}
