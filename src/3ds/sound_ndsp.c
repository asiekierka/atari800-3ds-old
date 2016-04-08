/*
 * 3ds/sound_ndsp.c - Nintendo 3DS NDSP audio backend
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
#include <stdlib.h>
#include <string.h>

#include "atari.h"
#include "log.h"
#include "platform.h"
#include "sound.h"

#define EMULATOR
/* #define SOUND_DEBUG */

int N3DS_soundFreq;
ndspWaveBuf N3DS_audioBuf;
u8*	N3DS_audioData;
u32	N3DS_lastSample, N3DS_curSample, N3DS_fragSize, N3DS_fragFrames;
Sound_setup_t *N3DS_sound;

int PLATFORM_SoundSetup(Sound_setup_t *setup)
{
	setup->sample_size = 1;

	if (setup->frag_frames == 0)
	{
		/* Set frag_frames automatically. */
		unsigned int val = setup->frag_frames = setup->freq / 50;
		unsigned int pow_val = 1;
		while (val >>= 1)
			pow_val <<= 1;
		if (pow_val < setup->frag_frames)
			pow_val <<= 1;
		setup->frag_frames = pow_val;
	}

	N3DS_fragFrames = setup->frag_frames;
	N3DS_fragSize = setup->sample_size * setup->channels;

	N3DS_audioData = (u8*) linearAlloc(N3DS_fragFrames * N3DS_fragSize);
	N3DS_ClearAudioData();

#ifdef EMULATOR
	return FALSE;
#else
	if (R_FAILED(ndspInit()))
	{
		printf("NDSP initialization error!\n");
		return FALSE;
	}

	N3DS_sound = setup;
	PLATFORM_SoundContinue();

	return TRUE;
#endif
}


void N3DS_ClearAudioData(void)
{
	memset(N3DS_audioData, 0, N3DS_fragFrames * N3DS_fragSize);
	DSP_FlushDataCache(N3DS_audioData, N3DS_fragFrames * N3DS_fragSize);
}

void PLATFORM_SoundExit(void)
{
	linearFree(N3DS_audioData);
	ndspExit();
}

void PLATFORM_SoundPause(void)
{
	ndspChnWaveBufClear(0);
	N3DS_ClearAudioData();
}

void PLATFORM_SoundContinue(void)
{
	float mix[12];
	memset(mix, 0, sizeof(mix));
	mix[0] = mix[1] = 1.0f;

	ndspSetOutputMode(N3DS_sound->channels == 2 ? NDSP_OUTPUT_STEREO : NDSP_OUTPUT_MONO);
	ndspSetOutputCount(1);
	ndspSetMasterVol(1.0f);

	memset(&N3DS_audioBuf, 0, sizeof(ndspWaveBuf));
	ndspChnReset(0);
	ndspChnSetInterp(0, NDSP_INTERP_POLYPHASE);
	ndspChnSetRate(0, N3DS_sound->freq);
	ndspChnSetFormat(0, NDSP_CHANNELS(N3DS_sound->channels) | NDSP_ENCODING(N3DS_sound->sample_size == 2 ? NDSP_ENCODING_PCM16 : NDSP_ENCODING_PCM8));
	ndspChnSetMix(0, mix);

	N3DS_audioBuf.data_vaddr = N3DS_audioData;
	N3DS_audioBuf.nsamples = N3DS_fragFrames;
	N3DS_audioBuf.looping = true;
	N3DS_audioBuf.status = NDSP_WBUF_FREE;

	N3DS_ClearAudioData();

	ndspChnWaveBufAdd(0, &N3DS_audioBuf);

	N3DS_lastSample = 0;
	N3DS_curSample = N3DS_fragFrames / 2;
}

#ifdef SOUND_DEBUG
int psaflag = 0;

unsigned int PLATFORM_SoundAvailable(void)
{
	return (((psaflag++) % 2) == 1) ? 300 : 0;
}
#else
unsigned int PLATFORM_SoundAvailable(void)
{
	u32 curSample = ndspChnGetSamplePos(0);
	u32 sampleCnt = curSample < N3DS_lastSample ? curSample + N3DS_fragFrames - N3DS_lastSample : curSample - N3DS_lastSample;
	return sampleCnt * N3DS_fragSize;
}
#endif

void PLATFORM_SoundWrite(UBYTE const *buffer, unsigned int size)
{
	u32 curSample = ndspChnGetSamplePos(0);

	int written = 0;
	int toWrite = size / N3DS_fragSize;
	int maxLen = N3DS_fragFrames;

	while (written < toWrite)
	{
		int canWrite = maxLen - N3DS_curSample;
		if (canWrite > (toWrite - written))
			canWrite = (toWrite - written);

		memcpy(&N3DS_audioData[N3DS_curSample], &buffer[written], canWrite * N3DS_fragSize);
		written += canWrite;
		N3DS_curSample = (N3DS_curSample + canWrite) % maxLen;
	}

	N3DS_lastSample = curSample;
}
