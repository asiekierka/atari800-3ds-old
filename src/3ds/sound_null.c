/*
 * sdl/sound.c - SDL library specific port code - sound output
 *
 * Copyright (c) 2001-2002 Jacek Poplawski
 * Copyright (C) 2001-2013 Atari800 development team (see DOC/CREDITS)
 *
 * This file is part of the Atari800 emulator project which emulates
 * the Atari 400, 800, 800XL, 130XE, and 5200 8-bit computers.
 *
 * Atari800 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Atari800 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Atari800; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <3ds.h>

#include "atari.h"
#include "log.h"
#include "platform.h"
#include "sound.h"

int PLATFORM_SoundSetup(Sound_setup_t *setup)
{
	return TRUE;
}

void PLATFORM_SoundExit(void)
{
}

void PLATFORM_SoundPause(void)
{
}

void PLATFORM_SoundContinue(void)
{
}

unsigned int PLATFORM_SoundAvailable(void)
{
	return 0;
}

void PLATFORM_SoundWrite(UBYTE const *buffer, unsigned int size)
{
}
