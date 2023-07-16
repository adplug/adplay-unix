/*
 * AdPlay/UNIX - OPL2 audio player
 * Copyright (C) 2023 Stian Skjelstad <stian.skjelstad@gmail.com>
 *
 * Code from https://github.com/SudoMaker/RetroWave is used as reference
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
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include "defines.h"
#include "retrowave.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/file.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#ifdef __APPLE__
#include <IOKit/serial/ioss.h>
#endif

#if defined (__CYGWIN__)
#define RETROWAVE_PLAYER_TIME_REF       CLOCK_REALTIME
#elif defined (__linux)
#define RETROWAVE_PLAYER_TIME_REF       CLOCK_MONOTONIC_RAW
#else
#define RETROWAVE_PLAYER_TIME_REF       CLOCK_MONOTONIC
#endif

typedef enum {
	RetroWave_Board_Unknown = 0,
	RetroWave_Board_OPL3 = 0x21 << 1,
	RetroWave_Board_MiniBlaster = 0x20 << 1,
	RetroWave_Board_MasterGear = 0x24 << 1
} RetroWaveBoardType;

static RetroWaveOpl *instance;

static void retrowave_sighandler (int ignore)
{
	static int recursive = 0;

	if (!recursive)
	{
		recursive = 1;
		if (instance)
		{
			instance->reset();
		}
		_exit(0);
	}
}

RetroWaveOpl::RetroWaveOpl(const char *filename)
{
	struct termios tio;
	currType = TYPE_OPL3;
	fd = open(filename, O_RDWR);
	if (fd < 0)
	{
		fprintf(stderr, "Failed to open tty/serial device %s: %s\n", filename, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (flock(fd, LOCK_EX))
	{
		fprintf(stderr, "Failed to lock tty/serial device: %s: %s\n", filename, strerror(errno));
		close(fd);
		fd = -1;
		exit(EXIT_FAILURE);
	}

	if (tcgetattr(fd, &tio))
	{
		fprintf(stderr, "Failed to perform tcgetattr() on device %s, not a tty/serial device?: %s\n", filename, strerror (errno));
		close(fd);
		fd = -1;
		exit(EXIT_FAILURE);
	}
	cfmakeraw(&tio);

#if 0 /* setting the speed appears to not be needed, and 2000000 is not valid on all operating systems */
#ifndef __APPLE__
	cfsetispeed(&tio, 2000000);
	cfsetospeed(&tio, 2000000);
#endif
#endif
	if (tcgetattr(fd, &tio))
	{
		fprintf(stderr, "Failed to perform tcsetattr() on device %s, not a tty/serial device?: %s\n", filename, strerror(errno));
		close(fd);
		fd = -1;
		exit(EXIT_FAILURE);
	}

#if 0 /* setting the speed appears to not be needed, and 2000000 is not valid on all operating systems */
#ifdef __APPLE__
	int speed = 2000000;

	if (ioctl(fd, IOSSIOSPEED, &speed) == -1)
	{
		fprintf(stderr, "Failed to set baudrate on device %s: %s", filename, strerror(errno));
		exit(EXIT_FAILURE);
	}
#endif
#endif

	cmd_buffer[0] = 0x00;
	cmd_buffer_used=1;
	io_prepare();
	flush();

	/* GPIOA.0 = /IC  Initial clear (Reset)
	 * GPIOA.1 = A0   Low=Address, High=Data
	 * GPIOA.2 = A1   Low=Bank0,   High=Bank1
	 * GPIOA.3 = /WR  Write enable
	 * GPIOA.4 = /CS  Chip Select
	 * GPIOA.5 =
	 * GPIOA.6 =
	 * GPIOA.7 =
	 * GPIOB[0:7] = D[0:7]
	 */

	for (uint8_t i=0x20; i<0x28; i++)
	{
		cmd_prepare((uint8_t)(i<<1), 0x0a, 1); // IOCON register
		cmd_buffer[cmd_buffer_used++] = 0x28;  // HAEN=1 SEQOP=1 BANK=0
		io_prepare();
		flush();

		cmd_prepare((uint8_t)(i<<1), 0x00, 2); // IODIRA register
		cmd_buffer[cmd_buffer_used++] = 0x00;  // Set output
		cmd_buffer[cmd_buffer_used++] = 0x00;  // Set output
		io_prepare();
		flush();

		cmd_prepare((uint8_t)(i<<1), 0x12, 2); // GPIOA register
		cmd_buffer[cmd_buffer_used++] = 0xff;  // Set all HIGH
		cmd_buffer[cmd_buffer_used++] = 0xff;  // Set all HIGH
		cmd_buffer_used = 4;
		io_prepare();
		flush();
	}
}

RetroWaveOpl::~RetroWaveOpl()
{
	if (fd >= 0)
	{
		close(fd);
		fd = -1;
	}
}

void RetroWaveOpl::cmd_prepare(uint8_t io_addr, uint8_t io_reg, const int len)
{
	if ((cmd_buffer_used > (8192-len))||
	    (cmd_buffer_used && (cmd_buffer[0] != io_addr)) ||
	    (cmd_buffer_used && (cmd_buffer[1] != io_reg)) )
	{
		io_prepare();
		flush();
	}

	if (!cmd_buffer_used)
	{
		cmd_buffer[cmd_buffer_used++] = io_addr;
		cmd_buffer[cmd_buffer_used++] = io_reg;
	}
}

void RetroWaveOpl::queue_port0(uint8_t reg, uint8_t val)
{
	cmd_prepare(RetroWave_Board_OPL3, 0x12, 6); // GPIOA register
	cmd_buffer[cmd_buffer_used++] = 0xe1;
	cmd_buffer[cmd_buffer_used++] = reg;
	cmd_buffer[cmd_buffer_used++] = 0xe3;
	cmd_buffer[cmd_buffer_used++] = val;
	cmd_buffer[cmd_buffer_used++] = 0xfb;
	cmd_buffer[cmd_buffer_used++] = val; // Retrowave express OPL3 seems to only like even data writes
}

void RetroWaveOpl::queue_port1(uint8_t reg, uint8_t val)
{
	cmd_prepare(RetroWave_Board_OPL3, 0x12, 6); // GPIOA register
	cmd_buffer[cmd_buffer_used++] = 0xe5;
	cmd_buffer[cmd_buffer_used++] = reg;
	cmd_buffer[cmd_buffer_used++] = 0xe7;
	cmd_buffer[cmd_buffer_used++] = val;
	cmd_buffer[cmd_buffer_used++] = 0xfb;
	cmd_buffer[cmd_buffer_used++] = val; // Retrowave express OPL3 seems to only like even data writes
}

void RetroWaveOpl::reset(void)
{
	if (cmd_buffer_used)
	{
		io_prepare();
		flush();
	}

#if 0 // reset by /IC, makes click sound
	cmd_prepare(RetroWave_Board_OPL3, 0x12, 1); // GPIOA register
	cmd_buffer[cmd_buffer_used++] = 0xfe;
	cmd_buffer[cmd_buffer_used++] = 0x00; // Retrowave express OPL3 seems to only like even data writes
	io_prepare();
	flush();

	usleep (1700); /* chip needs about 1.6ms to safely reset, so delay 1.7ms */

	cmd_prepare(RetroWave_Board_OPL3, 0x12, 1); // GPIOA register
	cmd_buffer[cmd_buffer_used++] = 0xff;
	cmd_buffer[cmd_buffer_used++] = 0x00; // Retrowave express OPL3 seems to only like even data writes
	io_prepare();
	flush();
#else // reset by registers
	queue_port1 (5, 1); // Enable OPL3 mode
	queue_port1 (4, 0); // Disable all 4-OP connections

	for (int i=0x20; i <= 0x35; i++)
	{
		queue_port0 (i, 0x01);
		queue_port1 (i, 0x01);
	}
	for (int i=0x40; i <= 0x55; i++)
	{
		queue_port0 (i, 0x3f);
		queue_port1 (i, 0x3f);
	}
	for (int i=0x60; i <= 0x75; i++)
	{
		queue_port0 (i, 0xee);
		queue_port1 (i, 0xee);
	}
	for (int i=0x80; i <= 0x95; i++)
	{
		queue_port0 (i, 0x0e);
		queue_port1 (i, 0x0e);
	}
	for (int i=0xa0; i <= 0xa8; i++)
	{
		queue_port0 (i, 0x80);
		queue_port1 (i, 0x80);
	}
	for (int i=0xb0; i <= 0xb8; i++)
	{
		queue_port0 (i, 0x04);
		queue_port1 (i, 0x04);
	}
	for (int i=0xbd; i <= 0xbd; i++)
	{
		queue_port0 (i, 0);
		queue_port1 (i, 0);
	}
	for (int i=0xc0; i <= 0xc8; i++)
	{
		queue_port0 (i, 0x30); // Enable Left and Right
		queue_port1 (i, 0x30); // Enable Left and Right
	}
	for (int i=0xe0; i <= 0xf5; i++)
	{
		queue_port0 (i, 0);
		queue_port1 (i, 0);
	}
	for (int i=0x08; i <= 0x08; i++)
	{
		queue_port0 (i, 0);
		queue_port1 (i, 0);
	}
	for (int i=0x01; i <= 0x01; i++)
	{
		queue_port0 (i, 0);
		queue_port1 (i, 0);
	}
	queue_port1 (5, 0); // OPL2 mode
	io_prepare();
	flush();
#endif
}

void RetroWaveOpl::io_prepare(void)
{
	uint_fast16_t data = 0;
	uint8_t fill = 0;
	io_buffer_used = 0;
	io_buffer[io_buffer_used++] = 0x00;

	if (!cmd_buffer_used)
	{
		return;
	}

	for (uint_fast16_t i=0; i < cmd_buffer_used;)
	{
		if (fill < 7)
		{
			data <<= 8;
			data |= cmd_buffer[i++];
			fill += 8;
		}
		io_buffer[io_buffer_used++] = ((data >> (fill - 7)) << 1) | 0x01;
		fill -= 7;
	}
	if (fill)
	{
		io_buffer[io_buffer_used++] = 0x01 | (data << 1);
	}

	io_buffer[io_buffer_used++] = 0x02;

	cmd_buffer_used = 0;
}

void RetroWaveOpl::flush(void)
{
	if (!io_buffer_used)
	{
		return;
	}
#if defined(__APPLE__) /* Atleast OS X El Capitan has issues with buffering causing corruption*/
        int pos = 0;
        while (pos < io_buffer_used)
        {
                int target = io_buffer_used - pos;
		if (target > 128) target = 128;
                int res;
                ::write(fd, io_buffer + pos, target);
                pos += target;
                usleep (250);
        }
#else
	::write(fd, io_buffer, io_buffer_used);
#endif
	io_buffer_used = 0;
}

void RetroWaveOpl::write(int reg, int val)
{
	if (currChip == 0)
	{
		queue_port0 (reg, val);
	} else if (currChip == 1)
	{
		queue_port1 (reg, val);
	}
}
void RetroWaveOpl::init(void)
{
	reset();
	currChip = 0;
}


RetroWavePlayer::RetroWavePlayer(const char *filename) : EmuPlayer(0, 16, 2, 44100, 65536)
{
	retrowaveopl = new RetroWaveOpl(filename ? filename : "/dev/ttyACM0");

	instance = retrowaveopl;
#ifdef SIGHUP
	signal (SIGHUP, retrowave_sighandler);
#endif
#ifdef SIGQUIT
	signal (SIGQUIT, retrowave_sighandler);
#endif
#ifdef SIGINT
	signal (SIGINT, retrowave_sighandler);
#endif
#ifdef SIGKILL
	signal (SIGKILL, retrowave_sighandler);
#endif

	clock_gettime(RETROWAVE_PLAYER_TIME_REF, &nexttick);
}

RetroWavePlayer::~RetroWavePlayer()
{
	delete retrowaveopl;
	instance = NULL;
	retrowaveopl = NULL;
}

Copl *RetroWavePlayer::get_opl(void)
{
	return retrowaveopl;
}

void RetroWavePlayer::frame()
{
	playing = p->update();
	retrowaveopl->io_prepare();
	retrowaveopl->flush();
	long long ticklen = 1000000000ll / p->getrefresh();
	struct timespec now;
	clock_gettime(RETROWAVE_PLAYER_TIME_REF, &now);
	nexttick.tv_nsec += ticklen;
	while (nexttick.tv_nsec > 1000000000)
	{
		nexttick.tv_sec++;
		nexttick.tv_nsec-= 1000000000;
	}
	if (nexttick.tv_sec < now.tv_sec)
	{
		return;
	}
	if ((nexttick.tv_sec == now.tv_sec) &&
	    (nexttick.tv_nsec < now.tv_nsec))
	{
		return;
	}
	usleep( (nexttick.tv_sec  - now.tv_sec ) * 1000000 +
	                 (nexttick.tv_nsec - now.tv_nsec) / 1000);
}
