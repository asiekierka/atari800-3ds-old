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

/* #define EMULATOR */
/* #define SOUND_DEBUG */

int N3DS_soundFreq;
ndspWaveBuf N3DS_audioBuf;
u8*	N3DS_audioData;
u32	N3DS_bufferSize, N3DS_sampleSize, N3DS_lastPos;
Sound_setup_t *N3DS_sound;

void N3DS_ClearAudioData(void);

int PLATFORM_SoundSetup(Sound_setup_t *setup)
{
	setup->sample_size = 1;

	if (setup->frag_frames == 0)
	{
		setup->frag_frames = 4096;
	}

	N3DS_bufferSize = setup->frag_frames;
	N3DS_sampleSize = setup->sample_size * setup->channels;

	N3DS_audioData = (u8*) linearAlloc(N3DS_bufferSize * 2 * N3DS_sampleSize);
	N3DS_ClearAudioData();

	ndspInit();

	N3DS_sound = setup;
	PLATFORM_SoundContinue();

	return TRUE;
}

void N3DS_SoundCallback(void* dud)
{
	u32 pos = ndspChnGetSamplePos(0);
	if (pos >= N3DS_bufferSize && N3DS_lastPos < N3DS_bufferSize)
	{
		Sound_Callback(&N3DS_audioData[0], N3DS_bufferSize * N3DS_sampleSize);
		for(int i = 0; i < N3DS_bufferSize * N3DS_sampleSize; i++)
			N3DS_audioData[i] ^= 0x80;
	}
	else if (pos < N3DS_bufferSize && N3DS_lastPos >= N3DS_bufferSize)
	{
		Sound_Callback(&N3DS_audioData[N3DS_bufferSize * N3DS_sampleSize], N3DS_bufferSize * N3DS_sampleSize);
		for(int i = N3DS_bufferSize * N3DS_sampleSize; i < N3DS_bufferSize * N3DS_sampleSize * 2; i++)
			N3DS_audioData[i] ^= 0x80;
	}
	N3DS_lastPos = pos;
}

void N3DS_ClearAudioData(void)
{
	memset(N3DS_audioData, 0, N3DS_bufferSize * 2 * N3DS_sampleSize);
	DSP_FlushDataCache(N3DS_audioData, N3DS_bufferSize * 2 * N3DS_sampleSize);
	N3DS_lastPos = 0;
}

void PLATFORM_SoundLock(void)
{
	// TODO
}

void PLATFORM_SoundUnlock(void)
{
	// TODO
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
	ndspSetCallback(N3DS_SoundCallback, N3DS_audioData);

	memset(&N3DS_audioBuf, 0, sizeof(ndspWaveBuf));
	ndspChnReset(0);
	ndspChnSetInterp(0, NDSP_INTERP_POLYPHASE);
	ndspChnSetRate(0, N3DS_sound->freq);
	ndspChnSetFormat(0, NDSP_CHANNELS(N3DS_sound->channels) | NDSP_ENCODING(N3DS_sound->sample_size == 2 ? NDSP_ENCODING_PCM16 : NDSP_ENCODING_PCM8));
	ndspChnSetMix(0, mix);

	N3DS_audioBuf.data_vaddr = N3DS_audioData;
	N3DS_audioBuf.nsamples = N3DS_bufferSize * 2;
	N3DS_audioBuf.looping = true;
	N3DS_audioBuf.status = NDSP_WBUF_FREE;

	N3DS_ClearAudioData();

	ndspChnWaveBufAdd(0, &N3DS_audioBuf);
}

// Pre-callback code

/* #ifdef SOUND_DEBUG
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
} */
