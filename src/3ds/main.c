/*
 * sdl/main.c - SDL library specific port code - main interface
 *
 * Copyright (c) 2001-2002 Jacek Poplawski
 * Copyright (C) 2001-2014 Atari800 development team (see DOC/CREDITS)
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

/*
   Thanks to David Olofson for scaling tips!

   TODO:
   - implement all Atari800 parameters
   - turn off fullscreen when error happen
*/

#include "config.h"
#include <3ds.h>
#include <sf2d.h>
#include <stdio.h>
#include <string.h>

/* Atari800 includes */
#include "atari.h"
#include "../input.h"
#include "log.h"
#include "monitor.h"
#include "platform.h"
#ifdef SOUND
#include "../sound.h"
#endif
#ifdef USE_UI_BASIC_ONSCREEN_KEYBOARD
#include "akey.h"
#include "ui_basic.h"
#endif
#include "videomode.h"

int PLATFORM_Configure(char *option, char *parameters)
{
	return 0;
	/* return SDL_VIDEO_ReadConfig(option, parameters) ||
	       SDL_INPUT_ReadConfig(option, parameters); */
}

void PLATFORM_ConfigSave(FILE *fp)
{
	/* SDL_VIDEO_WriteConfig(fp);
	SDL_INPUT_WriteConfig(fp); */
}

int PLATFORM_Initialise(int *argc, char *argv[])
{
	return TRUE;
}

void PLATFORM_Sleep(double s)
{
	if (s > 0) {
		svcSleepThread(s * 1e9);
	}
}

double PLATFORM_Time(void)
{
	return osGetTime() * 1e-3;
}

int PLATFORM_Exit(int run_monitor)
{
	Log_flushlog();

	if (run_monitor) {
		return 1;
	} else {
		gfxExit();
	}

	return 0;
}

int main(int argc, char **argv)
{
	osSetSpeedupEnable(true);
	aptOpenSession();
	APT_SetAppCpuTimeLimit(80); // enables syscore usage
	aptCloseSession();

	/* initialise platform early on for console debug */
	//gfxInit(GSP_RGBA8_OES, GSP_BGR8_OES, false);
	//gfxSet3D(false);
	sf2d_init();
	//consoleInit(GFX_BOTTOM, NULL);
	//gfxSetDoubleBuffering(GFX_TOP, true);

	/* initialise Atari800 core */
	if (!Atari800_Initialise(&argc, argv))
		return 3;

	/* main loop */
	while (aptMainLoop()) {
		INPUT_key_code = PLATFORM_Keyboard();
		Atari800_Frame();
		if (Atari800_display_screen)
			PLATFORM_DisplayScreen();
	}
}

/*
vim:ts=4:sw=4:
*/
