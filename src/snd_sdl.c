#include "quakedef.h"

static dma_t the_shm;
static int snd_inited;

extern int desired_speed;
extern int desired_bits;

static void paint_audio(void *unused, Uint8 *stream, int len)
{
	(void)unused; // shhh compiler its ok
	if (shm) {
		shm->buffer = stream;
		shm->samplepos += len / (shm->samplebits / 8) / 2;
		// Check for samplepos overflow?
		S_PaintChannels(shm->samplepos);
	}
}

qboolean SNDDMA_Init()
{
	snd_inited = 0;
	// Set up the desired format 
	int i;
	SDL_AudioSpec desired, obtained;
	if ((i = COM_CheckParm("-sndspeed")) != 0)
		desired_speed = atoi(com_argv[i + 1]);
	desired.freq = desired_speed;
	if ((i = COM_CheckParm("-sndbits")) != 0)
		desired_bits = atoi(com_argv[i + 1]);
	switch (desired_bits) {
	case 8:
		desired.format = AUDIO_U8;
		break;
	case 16:
		desired.format = SDL_BYTEORDER == SDL_BIG_ENDIAN ?
			AUDIO_S16MSB : AUDIO_S16LSB;
		break;
	default:
		Con_Printf("Unknown number of audio bits: %d\n", desired_bits);
		return 0;
	}
	if ((i = COM_CheckParm("-sndmono")) != 0) {
		desired.channels = 1; // sounds busted, FIXME
	} else if ((i = COM_CheckParm("-sndstereo")) != 0)
		desired.channels = 2;
	else
		desired.channels = 2;
	desired.samples = 512;
	if ((i = COM_CheckParm("-sndsamples")) != 0)
		desired.samples = atoi(com_argv[i + 1]);
	desired.callback = paint_audio;
	if (SDL_OpenAudio(&desired, &obtained) < 0) {
		Con_Printf("Couldn't open SDL audio: %s\n", SDL_GetError());
		return 0;
	}
	switch (obtained.format) { // Make sure we can support the audio format
	case AUDIO_U8:
		break; // Supported 
	case AUDIO_S16LSB:
	case AUDIO_S16MSB:
		if (((obtained.format == AUDIO_S16LSB) &&
		     (SDL_BYTEORDER == SDL_LIL_ENDIAN)) ||
		    ((obtained.format == AUDIO_S16MSB) &&
		     (SDL_BYTEORDER == SDL_BIG_ENDIAN))) {
			break; // Supported 
		}
		/* Unsupported */ ;
		/* fallthrough */
	default:
		// Not supported -- force SDL to do our bidding 
		SDL_CloseAudio();
		if (SDL_OpenAudio(&desired, NULL) < 0) {
			Con_Printf("Couldn't open SDL audio: %s\n",
				   SDL_GetError());
			return 0;
		}
		memcpy(&obtained, &desired, sizeof(desired));
		break;
	}
	SDL_PauseAudio(0);
	shm = &the_shm; // Fill the audio DMA information block
	shm->splitbuffer = 0;
	shm->samplebits = (obtained.format & 0xFF);
	shm->speed = obtained.freq;
	shm->channels = obtained.channels;
	shm->samples = obtained.samples * shm->channels;
	shm->samplepos = 0;
	shm->submission_chunk = 1;
	shm->buffer = NULL;
	if ((i = COM_CheckParm("-sndpitch")) != 0)
		shm->speed *= ((float)atoi(com_argv[i + 1]) / 10);
	snd_inited = 1;
	return 1;
}

int SNDDMA_GetDMAPos()
{
	return shm->samplepos;
}

void SNDDMA_Shutdown()
{
	if (snd_inited) {
		SDL_CloseAudio();
		snd_inited = 0;
	}
}
