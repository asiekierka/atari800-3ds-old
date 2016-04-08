/*
 * sdl/input.c - SDL library specific port code - input device support
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

#include <3ds.h>

#include "config.h"
#include "akey.h"
#include "atari.h"
#include "colours.h"
#include "filter_ntsc.h"
#include "input.h"
#include "log.h"
#include "platform.h"
#include "pokey.h"
#include "ui.h"
#include "util.h"
#include "videomode.h"
#include "screen.h"
#ifdef USE_UI_BASIC_ONSCREEN_KEYBOARD
#include "ui_basic.h"
#endif

#include "kbd_mapping_bin.h"

int key_control;

int N3DS_IsControlPressed()
{
	return key_control;
}

void PLATFORM_SetJoystickKey(int joystick, int direction, int value)
{
}

void PLATFORM_GetJoystickKeyName(int joystick, int direction, char *buffer, int bufsize)
{
}

int PLATFORM_GetRawKey(void)
{
	return 0;
}

int PLATFORM_Keyboard(void)
{
	u32 kDown, kHeld, kUp;
	u32* kMapping;
	touchPosition pos;
	int posPixel;

	hidScanInput();
	kDown = hidKeysDown();
	kHeld = hidKeysHeld();
	kUp = hidKeysUp();

	if (UI_is_active)
	{
		if (kDown & KEY_A)
			return AKEY_RETURN;
		if (kDown & KEY_B)
			return AKEY_ESCAPE;
		if (kDown & KEY_X)
			return AKEY_SPACE;
		if (kDown & KEY_Y)
			return AKEY_BACKSPACE;
		if (kHeld & KEY_UP)
			return AKEY_UP;
		if (kHeld & KEY_LEFT)
			return AKEY_LEFT;
		if (kHeld & KEY_RIGHT)
			return AKEY_RIGHT;
		if (kHeld & KEY_DOWN)
			return AKEY_DOWN;
		if (kDown & KEY_L)
			return AKEY_COLDSTART;
		if (kDown & KEY_R)
			return AKEY_WARMSTART;
	}

	if (kHeld & KEY_START)
		INPUT_key_consol ^= INPUT_CONSOL_START;
	if (kHeld & KEY_SELECT)
		INPUT_key_consol ^= INPUT_CONSOL_SELECT;
	if (kHeld & KEY_R)
		INPUT_key_consol ^= INPUT_CONSOL_OPTION;
	if (kHeld & KEY_B)
		return AKEY_RETURN;
	if (kHeld & KEY_Y)
		return AKEY_SPACE;
	if (kHeld & KEY_L)
		return AKEY_UI;
	if (kHeld & KEY_DUP)
		return AKEY_UP;
	if (kHeld & KEY_DLEFT)
		return AKEY_LEFT;
	if (kHeld & KEY_DRIGHT)
		return AKEY_RIGHT;
	if (kHeld & KEY_DDOWN)
		return AKEY_DOWN;

	if (kDown & KEY_TOUCH)
	{
		hidTouchRead(&pos);
		kMapping = (u32*) kbd_mapping_bin;
		if (pos.px >= 0 && pos.px < 320 && pos.py >= 0 && pos.py < 240)
		{
			posPixel = kMapping[pos.py * 320 + pos.px] & 0xFFFFFF;
			if ((posPixel & 0xff00ff) == 0)
			{
				int val = (posPixel >> 10) | (INPUT_key_shift != 0 ? AKEY_SHFT : 0)
					| (key_control != 0 ? AKEY_CTRL : 0);

				if (INPUT_key_shift != 0 || key_control != 0)
				{
					INPUT_key_shift = 0;
					key_control = 0;
					PLATFORM_DisplayScreen();
				}

				return val;
			}

			if (posPixel == 0xff00ff || posPixel == 0x7f007f)
				if (INPUT_key_shift != 0 || key_control != 0)
				{
					INPUT_key_shift = 0;
					key_control = 0;
					PLATFORM_DisplayScreen();
				}

			if (posPixel == 0x2020ff)
				INPUT_key_consol ^= INPUT_CONSOL_START;
			if (posPixel == 0x4040ff)
				INPUT_key_consol ^= INPUT_CONSOL_SELECT;
			if (posPixel == 0x6060ff)
				INPUT_key_consol ^= INPUT_CONSOL_OPTION;
			if (posPixel == 0x8080ff)
			{
				key_control = 1;
				PLATFORM_DisplayScreen();
			}
			if (posPixel == 0xa0a0ff)
			{
				INPUT_key_shift = 1;
				PLATFORM_DisplayScreen();
			}
			if (posPixel == 0xe0e0ff)
				return AKEY_BREAK;
		}
	}

	return AKEY_NONE;
}

int PLATFORM_PORT(int num)
{
	int ret = 0xff;
	if (num == 0) {
		hidScanInput();
		u32 kDown = hidKeysHeld();
		if (kDown & KEY_CPAD_LEFT)
			ret &= 0xf0 | INPUT_STICK_LEFT;
		if (kDown & KEY_CPAD_RIGHT)
			ret &= 0xf0 | INPUT_STICK_RIGHT;
		if (kDown & KEY_CPAD_UP)
			ret &= 0xf0 | INPUT_STICK_FORWARD;
		if (kDown & KEY_CPAD_DOWN)
			ret &= 0xf0 | INPUT_STICK_BACK;
	}
	return ret;
}

int PLATFORM_TRIG(int num)
{
	if (num == 0) {
		hidScanInput();
		if (hidKeysHeld() & KEY_A)
			return 0;
	}
	return 1;
}
