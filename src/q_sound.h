// Copyright (C) 1996-2001 Id Software, Inc.
// Copyright (C) 2002-2009 John Fitzgibbons and others
// Copyright (C) 2007-2008 Kristian Duske
// Copyright (C) 2010-2014 QuakeSpasm developers
// GPLv3 See LICENSE for details.

// sound.h -- client sound i/o functions

#ifndef __QUAKE_SOUND__
#define __QUAKE_SOUND__

typedef struct
{
	int left;
	int right;
} portable_samplepair_t;

typedef struct sfx_s
{
	char	name[MAX_QPATH];
	cache_user_t	cache;
} sfx_t;

typedef struct
{
	int	length;
	int	loopstart;
	int	speed;
	int	width;
	int	stereo;
	byte	data[1];	/* variable sized	*/
} sfxcache_t;

typedef struct
{
	int	channels;
	int	samples;		/* mono samples in buffer			*/
	int	submission_chunk;	/* don't mix less than this #			*/
	int	samplepos;		/* in mono samples				*/
	int	samplebits;
	int	signed8;		/* device opened for S8 format? (e.g. Amiga AHI) */
	int	speed;
	unsigned char	*buffer;
} dma_t;

typedef struct
{
	sfx_t	*sfx;			/* sfx number					*/
	int	leftvol;		/* 0-255 volume					*/
	int	rightvol;		/* 0-255 volume					*/
	int	end;			/* end time in global paintsamples		*/
	int	pos;			/* sample position in sfx			*/
	int	looping;		/* where to loop, -1 = no looping		*/
	int	entnum;			/* to allow overriding a specific sound		*/
	int	entchannel;
	vec3_t	origin;			/* origin of sound effect			*/
	vec_t	dist_mult;		/* distance multiplier (attenuation/clipK)	*/
	int	master_vol;		/* 0-255 master volume				*/
} channel_t;


typedef struct
{
	int	rate;
	int	width;
	int	channels;
	int	loopstart;
	int	samples;
	int	dataofs;		/* chunk starts this many bytes from file start	*/
} wavinfo_t;

void S_Init (void);
void S_Startup (void);
void S_Shutdown (void);
void S_StartSound (int entnum, int entchannel, sfx_t *sfx, vec3_t origin, float fvol, float attenuation);
void S_StaticSound (sfx_t *sfx, vec3_t origin, float vol, float attenuation);
void S_StopSound (int entnum, int entchannel);
void S_StopAllSounds(bool clear);
void S_ClearBuffer (void);
void S_Update (vec3_t origin, vec3_t forward, vec3_t right, vec3_t up);
void S_ExtraUpdate (void);

void S_BlockSound (void);
void S_UnblockSound (void);

sfx_t *S_PrecacheSound (const char *sample);
void S_TouchSound (const char *sample);
void S_ClearPrecache (void);
void S_BeginPrecaching (void);
void S_EndPrecaching (void);
void S_PaintChannels (int endtime);
void S_InitPaintChannels (void);

/* picks a channel based on priorities, empty slots, number of channels */
channel_t *SND_PickChannel (int entnum, int entchannel);

/* spatializes a channel */
void SND_Spatialize (channel_t *ch);

/* music stream support */
void S_RawSamples(int samples, int rate, int width, int channels, byte * data, float volume);
				/* Expects data in signed 16 bit, or unsigned 8 bit format. */

/* initializes cycling through a DMA buffer and returns information on it */
bool SNDDMA_Init(dma_t *dma);

/* gets the current DMA position */
int SNDDMA_GetDMAPos(void);

/* shutdown the DMA xfer. */
void SNDDMA_Shutdown(void);

/* validates & locks the dma buffer */
void SNDDMA_LockBuffer(void);

/* unlocks the dma buffer / sends sound to the device */
void SNDDMA_Submit(void);

/* blocks sound output upon window focus loss */
void SNDDMA_BlockSound(void);

/* unblocks the output upon window focus gain */
void SNDDMA_UnblockSound(void);

/* ====================================================================
 * User-setable variables
 * ====================================================================
 */


extern	channel_t	snd_channels[MAX_CHANNELS];
/* 0 to MAX_DYNAMIC_CHANNELS-1	= normal entity sounds
 * MAX_DYNAMIC_CHANNELS to MAX_DYNAMIC_CHANNELS + NUM_AMBIENTS -1 = water, etc
 * MAX_DYNAMIC_CHANNELS + NUM_AMBIENTS to total_channels = static sounds
 */

extern	volatile dma_t	*shm;

extern	int		total_channels;
extern	int		soundtime;
extern	int		paintedtime;
extern	int		s_rawend;

extern	vec3_t		listener_origin;
extern	vec3_t		listener_forward;
extern	vec3_t		listener_right;
extern	vec3_t		listener_up;


extern	portable_samplepair_t	s_rawsamples[MAX_RAW_SAMPLES];


void S_LocalSound (const char *name);
sfxcache_t *S_LoadSound (sfx_t *s);

wavinfo_t GetWavinfo (const char *name, byte *wav, int wavlength);

void SND_InitScaletable (void);

#endif	/* __QUAKE_SOUND__ */

