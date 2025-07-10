// Copyright (C) 1996-2001 Id Software, Inc.
// Copyright (C) 2002-2009 John Fitzgibbons and others
// Copyright (C) 2010-2014 QuakeSpasm developers
// GPLv3 See LICENSE for details.
#include "quakedef.h"

// FIXME: add to globals.h
u8 *COM_LoadTempFile(const s8 *path, u32 *path_id);

static Mix_Music *current_music = NULL;

void CDAudio_Play(u8 track, bool looping)
{
	char filename[MAX_QPATH];
	u8 *file = NULL;
	SDL_IOStream *io = NULL;
	Mix_Music *music = NULL;

	q_snprintf(filename, sizeof(filename), "music/track%02d.ogg", (int)track);

	file = COM_LoadTempFile(filename, NULL);
	if (!file)
	{
		q_snprintf(filename, sizeof(filename), "music/track%02d.mp3", (int)track);
		file = COM_LoadTempFile(filename, NULL);
		if (!file)
		{
			Con_Printf("file for track %02d not found\n", (int)track);
			return;
		}
	}

	io = SDL_IOFromConstMem(file, com_filesize);
	if (!io)
	{
		Con_Printf("failed to create IOStream for track %02d: %s\n", (int)track, SDL_GetError());
		return;
	}

	music = Mix_LoadMUS_IO(io, true);
	if (!music)
	{
		Con_Printf("failed to load track %02d: %s\n", (int)track, SDL_GetError());
		return;
	}

	CDAudio_Stop();

	current_music = music;

	if (!Mix_PlayMusic(current_music, looping ? -1 : 0))
	{
		Con_Printf("failed to play track %02d: %s\n", (int)track, SDL_GetError());
		return;
	}
}

void CDAudio_Stop()
{
	if (current_music)
		Mix_FreeMusic(current_music);
	current_music = NULL;
}

void CDAudio_Pause()
{
	Mix_PauseMusic();
}

void CDAudio_Resume()
{
	Mix_ResumeMusic();
}

void CDAudio_Update()
{
	static float last_volume = 0;

	if (bgmvolume.value < 0)
		Cvar_SetQuick(&bgmvolume, "0");
	if (bgmvolume.value > 1)
		Cvar_SetQuick(&bgmvolume, "1");

	if (last_volume != bgmvolume.value)
		Mix_VolumeMusic(bgmvolume.value * MIX_MAX_VOLUME);
}

bool CDAudio_Init()
{
	// FIXME: more formats? other ones that mod care about?
	MIX_InitFlags initflags = Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_OPUS);
	if (!(initflags & MIX_INIT_MP3) || !(initflags & MIX_INIT_OGG) || !(initflags & MIX_INIT_OPUS))
	{
		Con_Printf("Failed to initialize SDL Mixer: %s\n", SDL_GetError());
		return false;
	}

	if (!Mix_OpenAudio(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL))
	{
		Con_Printf("Failed to initialize SDL Mixer: %s\n", SDL_GetError());
		return false;
	}

	Con_Printf("SDL Mixer initialized\n");

	return true;
}

void CDAudio_Shutdown()
{
	CDAudio_Stop();
	Mix_Quit();
}
