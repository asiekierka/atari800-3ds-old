/*
 * sdl/video.c - SDL library specific port code - video display
 *
 * Copyright (c) 2001-2002 Jacek Poplawski
 * Copyright (C) 2001-2010 Atari800 development team (see DOC/CREDITS)
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
#include <sf2d.h>

#include "artifact.h"
#include "atari.h"
#include "colours.h"
#include "config.h"
#include "filter_ntsc.h"
#include "input.h"
#include "log.h"
#ifdef PAL_BLENDING
#include "pal_blending.h"
#endif /* PAL_BLENDING */
#include "platform.h"
#include "screen.h"
#include "util.h"
#include "videomode.h"

#include "kbd_ctrl_pressed_bin.h"
#include "kbd_shift_pressed_bin.h"
#include "kbd_display_bin.h"

int texInitialized = 0;
sf2d_texture *tex, *kbd_display, *kbd_ctrl_pressed, *kbd_shift_pressed;
u32 *texBuf;
VIDEOMODE_MODE_t N3DS_VIDEO_mode;
int ctable[256];

//#define NO_KEYBOARD_RENDER

void PLATFORM_PaletteUpdate(void)
{
#ifdef PAL_BLENDING
	if (N3DS_VIDEO_mode == VIDEOMODE_MODE_NORMAL && ARTIFACT_mode == ARTIFACT_PAL_BLEND)
		PAL_BLENDING_UpdateLookup();
#endif
	N3DS_VIDEO_PaletteUpdate();
}

void N3DS_VIDEO_PaletteUpdate()
{
	int i;

	for (i = 0; i < 256; i++)
	{
		ctable[i] = Colours_table[i] << 8 | 0xFF;
	}
}

void PLATFORM_GetPixelFormat(PLATFORM_pixel_format_t *format)
{
	format->bpp = 32;
	format->rmask = 0xFF000000;
	format->gmask = 0x00FF0000;
	format->bmask = 0x0000FF00;
}

void PLATFORM_MapRGB(void *dest, int const *palette, int size)
{
	int i;
	u32* target = (u32*) dest;
	for (i = 0; i < 256; i++)
	{
		target[i] = (palette[i] << 8) | 0xFF;
	}
}

static void UpdateNtscFilter(VIDEOMODE_MODE_t mode)
{
/*	if (mode != VIDEOMODE_MODE_NTSC_FILTER && FILTER_NTSC_emu != NULL) {
		FILTER_NTSC_Delete(FILTER_NTSC_emu);
		FILTER_NTSC_emu = NULL;
		N3DS_VIDEO_PaletteUpdate();
	}
	else if (mode == VIDEOMODE_MODE_NTSC_FILTER && FILTER_NTSC_emu == NULL) {
		FILTER_NTSC_emu = FILTER_NTSC_New();
		FILTER_NTSC_Update(FILTER_NTSC_emu);
	} */
}

void PLATFORM_SetVideoMode(VIDEOMODE_resolution_t const *res, int windowed, VIDEOMODE_MODE_t mode, int rotate90)
{
//	UpdateNtscFilter(mode);
	N3DS_VIDEO_mode = mode;
	N3DS_InitTexture();

	PLATFORM_PaletteUpdate();
	memset(texBuf, 0, tex->pow2_w * tex->pow2_h * 4);
	PLATFORM_DisplayScreen();
}

VIDEOMODE_resolution_t *PLATFORM_AvailableResolutions(unsigned int *size)
{
	VIDEOMODE_resolution_t *resolutions;
	resolutions = Util_malloc(1 * sizeof(VIDEOMODE_resolution_t));
	resolutions[0].width = 400;
	resolutions[0].height = 240;
	*size = 1;
	return resolutions;
}

VIDEOMODE_resolution_t *PLATFORM_DesktopResolution(void)
{
	VIDEOMODE_resolution_t *resolutions;
	resolutions = Util_malloc(1 * sizeof(VIDEOMODE_resolution_t));
	resolutions[0].width = 400;
	resolutions[0].height = 240;
	return resolutions;
}

int PLATFORM_SupportsVideomode(VIDEOMODE_MODE_t mode, int stretch, int rotate90)
{
	if (stretch != 0 || rotate90 != 0)
		return false;

	return mode == VIDEOMODE_MODE_NORMAL;
}

int PLATFORM_WindowMaximised(void)
{
	return 0;
}

void N3DS_RenderNormal(u8 *src, u32 *dest)
{
	int x, y;
	int spitch = Screen_WIDTH - VIDEOMODE_src_width;
	int dpitch = tex->pow2_w - VIDEOMODE_src_width;

	for (y = 0; y < VIDEOMODE_src_height; y++) {
		for (x = 0; x < VIDEOMODE_src_width; x++) {
			*(dest++) = ctable[*(src++)];
		}
		src += spitch;
		dest += dpitch;
	}
}

void N3DS_InitTexture(void)
{
	if (texInitialized == 0)
	{
		sf2d_set_3D(0);

		tex = sf2d_create_texture(Screen_WIDTH, Screen_HEIGHT, TEXFMT_RGBA8, SF2D_PLACE_RAM);
		texBuf = linearAlloc((tex->pow2_w * tex->pow2_h) << 2);

		kbd_display = sf2d_create_texture_mem_RGBA8(kbd_display_bin, 320, 240, TEXFMT_RGBA8, SF2D_PLACE_RAM);
		kbd_ctrl_pressed = sf2d_create_texture_mem_RGBA8(kbd_ctrl_pressed_bin, 320, 240, TEXFMT_RGBA8, SF2D_PLACE_RAM);
		kbd_shift_pressed = sf2d_create_texture_mem_RGBA8(kbd_shift_pressed_bin, 320, 240, TEXFMT_RGBA8, SF2D_PLACE_RAM);
		texInitialized = 1;
	}
}

void PLATFORM_DisplayScreen(void)
{
	u8 *src;
	u32 *dest;

	src = (u8*) Screen_atari;
	src += Screen_WIDTH * VIDEOMODE_src_offset_top + VIDEOMODE_src_offset_left;
	dest = &texBuf[VIDEOMODE_src_offset_top * tex->pow2_w + VIDEOMODE_src_offset_left];

	N3DS_InitTexture();

#ifdef PAL_BLENDING
	if (N3DS_VIDEO_mode == VIDEOMODE_MODE_NORMAL && ARTIFACT_mode == ARTIFACT_PAL_BLEND)
	{
		PAL_BLENDING_Blit32(dest, src, tex->pow2_w, VIDEOMODE_src_width, VIDEOMODE_src_height, VIDEOMODE_src_offset_top % 2);
	} else
#endif
		N3DS_RenderNormal(src, dest);

	//printf(".");

	sf2d_start_frame(GFX_TOP, GFX_LEFT);
	tex->tiled = 0;
	sf2d_texture_tile32_hardware(tex, texBuf, tex->pow2_w, tex->pow2_h);
	sf2d_draw_texture(tex, 8, 0);
	sf2d_end_frame();
#ifndef NO_KEYBOARD_RENDER
	sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
	sf2d_draw_texture(kbd_display, 0, 0);
	if (INPUT_key_shift != 0)
		sf2d_draw_texture(kbd_shift_pressed, 0, 0);
	if (N3DS_IsControlPressed() != 0)
		sf2d_draw_texture(kbd_ctrl_pressed, 0, 0);
	sf2d_end_frame();
#endif
	sf2d_swapbuffers();
}
