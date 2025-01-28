// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

// snd_mem.c: sound caching

#include "quakedef.h"

int cache_full_cycle;
byte *data_p;
byte *iff_end;
byte *last_chunk;
byte *iff_data;
int iff_chunk_len;

void ResampleSfx(sfx_t *sfx, int inrate, int inwidth, byte *data)
{
	sfxcache_t *sc = Cache_Check(&sfx->cache);
	if (!sc)
		return;
	float stepscale = (float)inrate / shm->speed; // this is usually 0.5, 1, or 2
	int outcount = sc->length / stepscale;
	sc->length = outcount;
	if (sc->loopstart != -1)
		sc->loopstart = sc->loopstart / stepscale;
	sc->speed = shm->speed;
	sc->width = loadas8bit.value ? 1 : inwidth;
	sc->stereo = 0;
	// resample / decimate to the current source rate
	if (stepscale == 1 && inwidth == 1 && sc->width == 1) {
		for (int i = 0; i < outcount; i++) // fast special case
			((char *)sc->data)[i]
			    = (int)((unsigned char)(data[i]) - 128);
	} else { // general case
		int samplefrac = 0;
		int fracstep = stepscale * 256;
		for (int i = 0; i < outcount; i++) {
			int srcsample = samplefrac >> 8;
			samplefrac += fracstep;
			int sample = (data[srcsample] - 128) << 8;
			if (inwidth == 2)
				sample= LittleShort(((short *)data)[srcsample]);
			if (sc->width == 2)
				((short *)sc->data)[i] = sample;
			else
				((char *)sc->data)[i] = sample >> 8;
		}
	}
}

sfxcache_t *S_LoadSound(sfx_t *s)
{
	char namebuffer[256];
	byte stackbuf[1 * 1024]; // avoid dirtying the cache heap
	sfxcache_t *sc = Cache_Check(&s->cache); // see if still in memory
	if (sc)
		return sc;
	Q_strcpy(namebuffer, "sound/"); // load it in
	Q_strcat(namebuffer, s->name);
	byte *data = COM_LoadStackFile(namebuffer, stackbuf, sizeof(stackbuf), NULL);
	if (!data) {
		Con_Printf("Couldn't load %s\n", namebuffer);
		return NULL;
	}
	wavinfo_t info = GetWavinfo(s->name, data, com_filesize);
	if (info.channels != 1) {
		Con_Printf("%s is a stereo sample\n", s->name);
		return NULL;
	}
	float stepscale = (float)info.rate / shm->speed;
	int len = info.samples / stepscale;
	len = len * info.width * info.channels;
	sc = Cache_Alloc(&s->cache, len + sizeof(sfxcache_t), s->name);
	if (!sc)
		return NULL;
	sc->length = info.samples;
	sc->loopstart = info.loopstart;
	sc->speed = info.rate;
	sc->width = info.width;
	sc->stereo = info.channels;
	ResampleSfx(s, sc->speed, sc->width, data + info.dataofs);
	return sc;
}

short GetLittleShort()
{
	short val = *data_p;
	val += *(data_p + 1) << 8;
	data_p += 2;
	return val;
}

int GetLittleLong()
{
	int val = *data_p;
	val += *(data_p + 1) << 8;
	val += *(data_p + 2) << 16;
	val += *(data_p + 3) << 24;
	data_p += 4;
	return val;
}

void FindNextChunk(char *name)
{
	while (1) {
		data_p = last_chunk;
		if (data_p >= iff_end) { // didn't find the chunk
			data_p = NULL;
			return;
		}
		data_p += 4;
		iff_chunk_len = GetLittleLong();
		if (iff_chunk_len < 0) {
			data_p = NULL;
			return;
		}
		data_p -= 8;
		last_chunk = data_p + 8 + ((iff_chunk_len + 1) & ~1);
		if (!Q_strncmp((const char *)data_p, name, 4))
			return;
	}
}

void FindChunk(char *name)
{
	last_chunk = iff_data;
	FindNextChunk(name);
}

wavinfo_t GetWavinfo(char *name, byte *wav, int wavlength)
{
	wavinfo_t info;
	memset(&info, 0, sizeof(info));
	if (!wav)
		return info;
	iff_data = wav;
	iff_end = wav + wavlength;
	FindChunk("RIFF");
	if (!(data_p && !Q_strncmp((const char *)data_p + 8, "WAVE", 4))) {
		Con_Printf("Missing RIFF/WAVE chunks\n");
		return info;
	}
	iff_data = data_p + 12;
	FindChunk("fmt ");
	if (!data_p) {
		Con_Printf("Missing fmt chunk\n");
		return info;
	}
	data_p += 8;
	int format = GetLittleShort();
	if (format != 1) {
		Con_Printf("Microsoft PCM format only\n");
		return info;
	}
	info.channels = GetLittleShort();
	info.rate = GetLittleLong();
	data_p += 4 + 2;
	info.width = GetLittleShort() / 8;
	FindChunk("cue ");
	if (data_p) {
		data_p += 32;
		info.loopstart = GetLittleLong();

		FindNextChunk("LIST"); // if the next chunk is a LIST chunk, look for a cue length marker
		if (data_p) {
			if (!strncmp((const char *)data_p + 28, "mark", 4)) { // this is not a proper parse, but it works with cooledit...
				data_p += 24;
				int i = GetLittleLong(); // samples in loop
				info.samples = info.loopstart + i;
			}
		}
	} else
		info.loopstart = -1;
	FindChunk("data");
	if (!data_p) {
		Con_Printf("Missing data chunk\n");
		return info;
	}
	data_p += 4;
	int samples = GetLittleLong() / info.width;
	if (info.samples) {
		if (samples < info.samples)
			Sys_Error("Sound %s has a bad loop length", name);
	} else
		info.samples = samples;
	info.dataofs = data_p - wav;
	return info;
}
