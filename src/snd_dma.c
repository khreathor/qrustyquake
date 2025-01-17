// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

// snd_dma.c -- main control for any streaming sound output device

#include "quakedef.h"

#define MAX_SFX 512

channel_t channels[MAX_CHANNELS];
int total_channels;
int snd_blocked = 0;
static qboolean snd_ambient = 1;
qboolean snd_initialized = false;
volatile dma_t *shm = 0; // pointer should go away
volatile dma_t sn;
vec3_t listener_origin;
vec3_t listener_forward;
vec3_t listener_right;
vec3_t listener_up;
vec_t sound_nominal_clip_dist = 1000.0;
int soundtime; // sample PAIRS
int paintedtime; // sample PAIRS
sfx_t *known_sfx; // hunk allocated [MAX_SFX]
int num_sfx;
sfx_t *ambient_sfx[NUM_AMBIENTS];
int desired_speed = 11025;
int desired_bits = 16;
int sound_started = 0;
// User-setable variables
// Fake dma is a synchronous faking of the DMA progress used for
// isolating performance in the renderer. The fakedma_updates is
// number of times S_Update() is called per second.
qboolean fakedma = false;
int fakedma_updates = 15;

cvar_t bgmvolume = { "bgmvolume", "1", true, false, 0, NULL };
cvar_t volume = { "volume", "0.7", true, false, 0, NULL };
cvar_t nosound = { "nosound", "0", false, false, 0, NULL };
cvar_t precache = { "precache", "1", false, false, 0, NULL };
cvar_t loadas8bit = { "loadas8bit", "0", false, false, 0, NULL };
cvar_t bgmbuffer = { "bgmbuffer", "4096", false, false, 0, NULL };
cvar_t ambient_level = { "ambient_level", "0.3", false, false, 0, NULL };
cvar_t ambient_fade = { "ambient_fade", "100", false, false, 0, NULL };
cvar_t snd_noextraupdate = { "snd_noextraupdate", "0", false, false, 0, NULL };
cvar_t snd_show = { "snd_show", "0", false, false, 0, NULL };
cvar_t _snd_mixahead = { "_snd_mixahead", "0.1", true, false, 0, NULL };

void S_Play();
void S_PlayVol();
void S_SoundList();
void S_StopAllSounds(qboolean clear);
void S_StopAllSoundsC();

void S_SoundInfo_f()
{
	if (!sound_started || !shm) {
		Con_Printf("sound system not started\n");
		return;
	}
	Con_Printf("%5d stereo\n", shm->channels - 1);
	Con_Printf("%5d samples\n", shm->samples);
	Con_Printf("%5d samplepos\n", shm->samplepos);
	Con_Printf("%5d samplebits\n", shm->samplebits);
	Con_Printf("%5d submission_chunk\n", shm->submission_chunk);
	Con_Printf("%5d speed\n", shm->speed);
	Con_Printf("0x%x dma buffer\n", shm->buffer);
	Con_Printf("%5d total_channels\n", total_channels);
}

void S_Startup()
{
	if (!snd_initialized)
		return;
	if (!fakedma) {
		int rc = SNDDMA_Init();
		if (!rc) {
			Con_Printf("S_Startup: SNDDMA_Init failed.\n");
			sound_started = 0;
			return;
		}
	}
	sound_started = 1;
}

void S_Init()
{
	Con_Printf("\nSound Initialization\n");
	if (COM_CheckParm("-nosound"))
		return;
	if (COM_CheckParm("-simsound"))
		fakedma = true;
	Cmd_AddCommand("play", S_Play);
	Cmd_AddCommand("playvol", S_PlayVol);
	Cmd_AddCommand("stopsound", S_StopAllSoundsC);
	Cmd_AddCommand("soundlist", S_SoundList);
	Cmd_AddCommand("soundinfo", S_SoundInfo_f);
	Cvar_RegisterVariable(&nosound);
	Cvar_RegisterVariable(&volume);
	Cvar_RegisterVariable(&precache);
	Cvar_RegisterVariable(&loadas8bit);
	Cvar_RegisterVariable(&bgmvolume);
	Cvar_RegisterVariable(&bgmbuffer);
	Cvar_RegisterVariable(&ambient_level);
	Cvar_RegisterVariable(&ambient_fade);
	Cvar_RegisterVariable(&snd_noextraupdate);
	Cvar_RegisterVariable(&snd_show);
	Cvar_RegisterVariable(&_snd_mixahead);
	if (host_parms.memsize < 0x800000) {
		Cvar_Set("loadas8bit", "1");
		Con_Printf("loading all sounds as 8bit\n");
	}
	snd_initialized = true;
	S_Startup();
	SND_InitScaletable();
	known_sfx = Hunk_AllocName(MAX_SFX * sizeof(sfx_t), "sfx_t");
	num_sfx = 0;
	if (fakedma) { // create a piece of DMA memory
		shm = (void *)Hunk_AllocName(sizeof(*shm), "shm");
		shm->splitbuffer = 0;
		shm->samplebits = 16;
		shm->speed = 22050;
		shm->channels = 2;
		shm->samples = 32768;
		shm->samplepos = 0;
		shm->soundalive = true;
		shm->gamealive = true;
		shm->submission_chunk = 1;
		shm->buffer = Hunk_AllocName(1 << 16, "shmbuf");
	}
	if (shm)
		Con_Printf("Sound sampling rate: %i\n", shm->speed);
	ambient_sfx[AMBIENT_WATER] = S_PrecacheSound("ambience/water1.wav");
	ambient_sfx[AMBIENT_SKY] = S_PrecacheSound("ambience/wind2.wav");
	S_StopAllSounds(true);
}

void S_Shutdown()
{
	if (!sound_started)
		return;
	if (shm)
		shm->gamealive = 0;
	shm = 0;
	sound_started = 0;
	if (!fakedma)
		SNDDMA_Shutdown();
}

sfx_t *S_FindName(char *name)
{
	if (!name)
		Sys_Error("S_FindName: NULL\n");
	if (Q_strlen(name) >= MAX_QPATH)
		Sys_Error("Sound name too long: %s", name);
	int i = 0;
	for (; i < num_sfx; i++) // see if already loaded
		if (!Q_strcmp(known_sfx[i].name, name))
			return &known_sfx[i];
	if (num_sfx == MAX_SFX)
		Sys_Error("S_FindName: out of sfx_t");
	sfx_t *sfx = &known_sfx[i];
	strcpy(sfx->name, name);
	num_sfx++;
	return sfx;
}

void S_TouchSound(char *name)
{
	if (!sound_started)
		return;
	sfx_t *sfx = S_FindName(name);
	Cache_Check(&sfx->cache);
}

sfx_t *S_PrecacheSound(char *name)
{
	if (!sound_started || nosound.value)
		return NULL;
	sfx_t *sfx = S_FindName(name);
	if (precache.value) // cache it in
		S_LoadSound(sfx);

	return sfx;
}

channel_t *SND_PickChannel(int entnum, int entchannel)
{
	// Check for replacement sound, or find the best one to replace
	int first_to_die = -1;
	int life_left = 0x7fffffff;
	for (int ch_idx = NUM_AMBIENTS;
	     ch_idx < NUM_AMBIENTS + MAX_DYNAMIC_CHANNELS; ch_idx++) {
		if (entchannel != 0	// channel 0 never overrides
			&& channels[ch_idx].entnum == entnum
			&& (channels[ch_idx].entchannel == entchannel
			|| entchannel == -1)) {	// allways override sound from same entity
			first_to_die = ch_idx;
			break;
		}
		// don't let monster sounds override player sounds
		if (channels[ch_idx].entnum == cl.viewentity
		    && entnum != cl.viewentity && channels[ch_idx].sfx)
			continue;
		if (channels[ch_idx].end - paintedtime < life_left) {
			life_left = channels[ch_idx].end - paintedtime;
			first_to_die = ch_idx;
		}
	}
	if (first_to_die == -1)
		return NULL;
	if (channels[first_to_die].sfx)
		channels[first_to_die].sfx = NULL;
	return &channels[first_to_die];
}

void SND_Spatialize(channel_t *ch)
{
	// anything coming from the view entity will allways be full volume
	if (ch->entnum == cl.viewentity) {
		ch->leftvol = ch->master_vol;
		ch->rightvol = ch->master_vol;
		return;
	}
	// calculate stereo seperation and distance attenuation
	vec3_t source_vec;
	VectorSubtract(ch->origin, listener_origin, source_vec);
	vec_t dist = VectorNormalize(source_vec) * ch->dist_mult;
	vec_t dot = DotProduct(listener_right, source_vec);
	vec_t rscale = 1.0 + (shm->channels == 1 ? 0 : dot);
	vec_t lscale = 1.0 - (shm->channels == 1 ? 0 : dot);
	vec_t scale = (1.0 - dist) * rscale; // add in distance effect
	ch->rightvol = (int)(ch->master_vol * scale);
	if (ch->rightvol < 0)
		ch->rightvol = 0;
	scale = (1.0 - dist) * lscale;
	ch->leftvol = (int)(ch->master_vol * scale);
	if (ch->leftvol < 0)
		ch->leftvol = 0;
}

void S_StartSound(int entnum, int entchannel, sfx_t *sfx, vec3_t origin,
		  float fvol, float attenuation)
{
	if (!sound_started || !sfx || nosound.value)
		return;
	int vol = fvol * 255;
	// pick a channel to play on
	channel_t *target_chan = SND_PickChannel(entnum, entchannel);
	if (!target_chan)
		return;
	memset(target_chan, 0, sizeof(*target_chan)); // spatialize
	VectorCopy(origin, target_chan->origin);
	target_chan->dist_mult = attenuation / sound_nominal_clip_dist;
	target_chan->master_vol = vol;
	target_chan->entnum = entnum;
	target_chan->entchannel = entchannel;
	SND_Spatialize(target_chan);

	if (!target_chan->leftvol && !target_chan->rightvol)
		return; // not audible at all
	sfxcache_t *sc = S_LoadSound(sfx); // new channel
	if (!sc) {
		target_chan->sfx = NULL;
		return; // couldn't load the sound's data
	}
	target_chan->sfx = sfx;
	target_chan->pos = 0.0;
	target_chan->end = paintedtime + sc->length;
	// if an identical sound has also been started this frame, offset the
	// pos a bit to keep it from just making the first one louder
	channel_t *check = &channels[NUM_AMBIENTS];
	for (int ch_idx = NUM_AMBIENTS;
		ch_idx < NUM_AMBIENTS+MAX_DYNAMIC_CHANNELS; ch_idx++, check++) {
		if (check == target_chan)
			continue;
		if (check->sfx == sfx && !check->pos) {
			int skip = rand() % (int)(0.1 * shm->speed);
			if (skip >= target_chan->end)
				skip = target_chan->end - 1;
			target_chan->pos += skip;
			target_chan->end -= skip;
			break;
		}
	}
}

void S_StopSound(int entnum, int entchannel)
{
	for (int i = 0; i < MAX_DYNAMIC_CHANNELS; i++) {
		if (channels[i].entnum == entnum
		    && channels[i].entchannel == entchannel) {
			channels[i].end = 0;
			channels[i].sfx = NULL;
			return;
		}
	}
}

void S_StopAllSounds(qboolean clear)
{
	if (!sound_started)
		return;
	total_channels = MAX_DYNAMIC_CHANNELS + NUM_AMBIENTS; // no statics
	for (int i = 0; i < MAX_CHANNELS; i++)
		if (channels[i].sfx)
			channels[i].sfx = NULL;
	Q_memset(channels, 0, MAX_CHANNELS * sizeof(channel_t));
	if (clear)
		S_ClearBuffer();
}

void S_StopAllSoundsC()
{
	S_StopAllSounds(true);
}

void S_ClearBuffer()
{
	if (!sound_started || !shm || !shm->buffer)
		return;
	int clear = shm->samplebits == 8 ? 0x80 : 0;
	Q_memset(shm->buffer, clear, shm->samples * shm->samplebits / 8);
}

void S_StaticSound(sfx_t *sfx, vec3_t origin, float vol, float attenuation)
{
	if (!sfx)
		return;
	if (total_channels == MAX_CHANNELS) {
		Con_Printf("total_channels == MAX_CHANNELS\n");
		return;
	}
	channel_t *ss = &channels[total_channels];
	total_channels++;
	sfxcache_t *sc = S_LoadSound(sfx);
	if (!sc)
		return;
	if (sc->loopstart == -1) {
		Con_Printf("Sound %s not looped\n", sfx->name);
		return;
	}
	ss->sfx = sfx;
	VectorCopy(origin, ss->origin);
	ss->master_vol = vol;
	ss->dist_mult = (attenuation / 64) / sound_nominal_clip_dist;
	ss->end = paintedtime + sc->length;
	SND_Spatialize(ss);
}

void S_UpdateAmbientSounds()
{
	if (!snd_ambient || !cl.worldmodel)
		return;
	// calc ambient sound levels
	mleaf_t *l = Mod_PointInLeaf(listener_origin, cl.worldmodel);
	if (!l || !ambient_level.value) {
		for (int amb_ch = 0; amb_ch < NUM_AMBIENTS; amb_ch++)
			channels[amb_ch].sfx = NULL;
		return;
	}
	for (int amb_ch = 0; amb_ch < NUM_AMBIENTS; amb_ch++) {
		channel_t *chan = &channels[amb_ch];
		chan->sfx = ambient_sfx[amb_ch];
		float vol = ambient_level.value*l->ambient_sound_level[amb_ch];
		if (vol < 8)
			vol = 0;
		if (chan->master_vol < vol) { // don't adjust volume too fast
			chan->master_vol += host_frametime * ambient_fade.value;
			if (chan->master_vol > vol)
				chan->master_vol = vol;
		} else if (chan->master_vol > vol) {
			chan->master_vol -= host_frametime * ambient_fade.value;
			if (chan->master_vol < vol)
				chan->master_vol = vol;
		}
		chan->leftvol = chan->rightvol = chan->master_vol;
	}
}


void S_Update(vec3_t origin, vec3_t forward, vec3_t right, vec3_t up)
{ // Called once each time through the main loop
	if (!sound_started || (snd_blocked > 0))
		return;
	VectorCopy(origin, listener_origin);
	VectorCopy(forward, listener_forward);
	VectorCopy(right, listener_right);
	VectorCopy(up, listener_up);
	S_UpdateAmbientSounds(); // update general area ambient sound sources
	channel_t *combine = NULL;
	// update spatialization for static and dynamic sounds  
	channel_t *ch = channels + NUM_AMBIENTS;
	for (int i = NUM_AMBIENTS; i < total_channels; i++, ch++) {
		if (!ch->sfx)
			continue;
		SND_Spatialize(ch); // respatialize channel
		if (!ch->leftvol && !ch->rightvol)
			continue;
		// try to combine static sounds with a previous channel of the
		// same sound effect so we don't mix five torches every frame
		if (i >= MAX_DYNAMIC_CHANNELS + NUM_AMBIENTS) {
			// see if it can just use the last one
			if (combine && combine->sfx == ch->sfx) {
				combine->leftvol += ch->leftvol;
				combine->rightvol += ch->rightvol;
				ch->leftvol = ch->rightvol = 0;
				continue;
			}
			// search for one
			combine = channels+MAX_DYNAMIC_CHANNELS+NUM_AMBIENTS;
			int j = MAX_DYNAMIC_CHANNELS + NUM_AMBIENTS;
			for (; j < i; j++, combine++)
				if (combine->sfx == ch->sfx)
					break;
			if (j == total_channels)
				combine = NULL;
			else {
				if (combine != ch) {
					combine->leftvol += ch->leftvol;
					combine->rightvol += ch->rightvol;
					ch->leftvol = ch->rightvol = 0;
				}
				continue;
			}
		}
	}
	if (snd_show.value) { // debugging output
		int total = 0;
		ch = channels;
		for (int i = 0; i < total_channels; i++, ch++)
			if (ch->sfx && (ch->leftvol || ch->rightvol))
				total++;
		Con_Printf("----(%i)----\n", total);
	}
}

void S_Play()
{
	static int hash = 345;
	char name[256];
	int i = 1;
	while (i < Cmd_Argc()) {
		if (!Q_strrchr(Cmd_Argv(i), '.')) {
			Q_strcpy(name, Cmd_Argv(i));
			Q_strcat(name, ".wav");
		} else
			Q_strcpy(name, Cmd_Argv(i));
		sfx_t *sfx = S_PrecacheSound(name);
		S_StartSound(hash++, 0, sfx, listener_origin, 1.0, 1.0);
		i++;
	}
}

void S_PlayVol()
{
	static int hash = 543;
	char name[256];
	int i = 1;
	while (i < Cmd_Argc()) {
		if (!Q_strrchr(Cmd_Argv(i), '.')) {
			Q_strcpy(name, Cmd_Argv(i));
			Q_strcat(name, ".wav");
		} else
			Q_strcpy(name, Cmd_Argv(i));
		sfx_t *sfx = S_PrecacheSound(name);
		float vol = Q_atof(Cmd_Argv(i + 1));
		S_StartSound(hash++, 0, sfx, listener_origin, vol, 1.0);
		i += 2;
	}
}

void S_SoundList()
{
	int total = 0;
	sfx_t *sfx = known_sfx; 
	for (int i = 0; i < num_sfx; i++, sfx++) {
		sfxcache_t *sc = Cache_Check(&sfx->cache);
		if (!sc)
			continue;
		int size = sc->length * sc->width * (sc->stereo + 1);
		total += size;
		if (sc->loopstart >= 0)
			Con_Printf("L");
		else
			Con_Printf(" ");
		Con_Printf("(%2db) %6i : %s\n", sc->width * 8, size, sfx->name);
	}
	Con_Printf("Total resident: %i\n", total);
}

void S_LocalSound(char *sound)
{
	if (nosound.value || !sound_started)
		return;
	sfx_t *sfx = S_PrecacheSound(sound);
	if (!sfx) {
		Con_Printf("S_LocalSound: can't cache %s\n", sound);
		return;
	}
	S_StartSound(cl.viewentity, -1, sfx, vec3_origin, 1, 1);
}
