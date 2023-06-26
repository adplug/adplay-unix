/*
 * AdPlay/UNIX - OPL2 audio player
 * Copyright (C) 2023 Stian Skjelstad <stian.skjelstad@gmail.com>
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

#ifndef H_RETROWAVE
#define H_RETROWAVE

#include <adplug/opl.h>
#include "output.h"

class RetroWaveOpl : public Copl
{
public:
	RetroWaveOpl(const char *filename);
	~RetroWaveOpl();

	void io_prepare(void);
	void flush(void);

protected:
	void queue_port0(uint8_t reg, uint8_t val);
	void queue_port1(uint8_t reg, uint8_t val);
	void reset(void);

private:
	int fd = -1;
	void cmd_prepare(uint8_t io_addr, uint8_t io_reg, const int len);
	uint8_t cmd_buffer[8192];
	uint_fast16_t cmd_buffer_used = 0;
	uint8_t io_buffer[9365];
	uint_fast16_t io_buffer_used;

	void write(int reg, int val);
	void init(void);
};

class RetroWavePlayer : public EmuPlayer
{
public:
	RetroWavePlayer(const char *filename);
	~RetroWavePlayer();

	void frame();
	Copl *get_opl(void);

protected:
	void output(const void *buf, unsigned long size) {};

private:
	struct timespec nexttick;
	RetroWaveOpl *retrowaveopl;
};

#endif
