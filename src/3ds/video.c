/*
 * 3ds/video.c - Nintendo 3DS video backend
 *
 * Copyright (c) 2001-2002 Jacek Poplawski
 * Copyright (C) 2001-2016 Atari800 development team (see DOC/CREDITS)
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
#include <sfil.h>

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

int texInitialized = 0;
sf2d_texture *tex, *kbd_display, *kbd_ctrl_pressed, *kbd_shift_pressed;
u32 *texBuf;
VIDEOMODE_MODE_t N3DS_VIDEO_mode;
int ctable[256];

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

}

void PLATFORM_SetVideoMode(VIDEOMODE_resolution_t const *res, int windowed, VIDEOMODE_MODE_t mode, int rotate90)
{
	N3DS_VIDEO_mode = mode;
	N3DS_InitTexture();

	PLATFORM_PaletteUpdate();
	memset(texBuf, 0, tex->tex.width * tex->tex.height * 4);
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
	if (rotate90 != 0)
		return false;

	return mode == VIDEOMODE_MODE_NORMAL;
}

int PLATFORM_WindowMaximised(void)
{
	return 1;
}

void N3DS_RenderNormal(u8 *src, u32 *dest)
{
	int x, y;
	int spitch = Screen_WIDTH - VIDEOMODE_src_width;
	int dpitch = 512 - VIDEOMODE_src_width;

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
		sf2d_swapbuffers();
		sf2d_set_3D(0);

		tex = sf2d_create_texture(512, 256, TEXFMT_RGBA8, SF2D_PLACE_RAM);
		texBuf = linearAlloc(512 * 256 * 4);

		kbd_display = sfil_load_PNG_file("romfs:/kbd_display.png", SF2D_PLACE_RAM);
		kbd_ctrl_pressed = sfil_load_PNG_file("romfs:/kbd_ctrl_pressed.png", SF2D_PLACE_RAM);
		kbd_shift_pressed = sfil_load_PNG_file("romfs:/kbd_shift_pressed.png", SF2D_PLACE_RAM);
		texInitialized = 1;
	}
}

void PLATFORM_DisplayScreen(void)
{
	u8 *src;
	u32 *dest;
	float xs, ys;

	src = (u8*) Screen_atari;
	src += Screen_WIDTH * VIDEOMODE_src_offset_top + VIDEOMODE_src_offset_left;
	dest = texBuf;

	N3DS_InitTexture();

#ifdef PAL_BLENDING
	if (N3DS_VIDEO_mode == VIDEOMODE_MODE_NORMAL && ARTIFACT_mode == ARTIFACT_PAL_BLEND)
		PAL_BLENDING_Blit32(dest, src, tex->tex.width, VIDEOMODE_src_width, VIDEOMODE_src_height, VIDEOMODE_src_offset_top % 2);
	else
#endif
		N3DS_RenderNormal(src, dest);

	sf2d_start_frame(GFX_TOP, GFX_LEFT);
	tex->tiled = 0;
	sf2d_texture_tile32_hardware(tex, texBuf, 512, 256);
	if (VIDEOMODE_dest_width <= 400 && VIDEOMODE_dest_height <= 240 && VIDEOMODE_src_width == VIDEOMODE_dest_width && VIDEOMODE_src_height == VIDEOMODE_dest_height)
		sf2d_draw_texture_part(tex,
			(400 - VIDEOMODE_dest_width) / 2, (240 - VIDEOMODE_dest_height) / 2,
			0, 0, VIDEOMODE_dest_width, VIDEOMODE_dest_height
		);
	else
	{
		xs = (float) VIDEOMODE_dest_width / VIDEOMODE_src_width;
		ys = (float) VIDEOMODE_dest_height / VIDEOMODE_src_height;
		sf2d_draw_texture_part_scale(tex,
			(400 - VIDEOMODE_dest_width) / 2, (240 - VIDEOMODE_dest_height) / 2,
			0, 0, VIDEOMODE_src_width, VIDEOMODE_src_height,
			xs, ys
		);
	}
	sf2d_end_frame();

	/* Keyboard rendering */
	sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
	sf2d_draw_texture(kbd_display, 0, 0);
	if (INPUT_key_shift != 0)
		sf2d_draw_texture(kbd_shift_pressed, 0, 0);
	if (N3DS_IsControlPressed() != 0)
		sf2d_draw_texture(kbd_ctrl_pressed, 0, 0);
	sf2d_end_frame();

	sf2d_swapbuffers();
}
