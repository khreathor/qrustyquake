#include "quakedef.h"
#include "d_local.h"

float *randarr; // used for noise bias
int fog_initialized = 0;
static unsigned int lfsr = 0x1337;

cvar_t fog = { "fog", "1", true, false, 0, NULL }; // CyanBun96

unsigned int lfsr_random() {
	lfsr ^= lfsr >> 7;
	lfsr ^= lfsr << 9;
	lfsr ^= lfsr >> 13;
	return lfsr;
}

float compute_fog(int z, float fog_density) {
	z /= 10;
	return expf(-fog_density * fog_density * (float)(z * z));
}

unsigned char sw_avg(unsigned char v1, float f) { // TODO colors?
	unsigned char retv;
	if (v1 < 0x80)
		retv = v1 & 0x0F;
	else if (v1 < 0xE0)
		retv = 0x10 - (v1 & 0x0F);
	else if (1 || v1 < 0xF0)
		retv = v1 & 0x0F;
	else retv = v1;
	if ((int)(f*16) >= retv)
		return 0;
	return retv - (int)(f*16);
}

int dither(int x, int y, float f) {
	int odd = y & 1;
	int threed = ((y % 3) == 0);
	f = 1 - f;
	if (f >= 1.00) return 0;
	else if (f > 0.83) { if(!(!(x%3) && odd)) return 0; }
	else if (f > 0.75) { if(!(x&1 && odd)) return 0; }
	else if (f > 0.66) { if(!(x%3 && odd)) return 0; }
	else if (f > 0.50) { if((x&1 && odd) || (!(x&1) && !odd)) return 0; }
	else if (f > 0.33) { if((x%3 && odd)) return 0; }
	else if (f > 0.25) { if(x&1 && odd) return 0; }
	else if (f > 0.16) { if(!(x%3) && odd) return 0; }
	else if (f > 0.04) { if(!(x%3) && odd && threed) return 0; }
	return 1; // don't draw
}

void R_InitFog()
{
	randarr = malloc(vid.width * vid.height * sizeof(float)); // TODO not optimal, use the zone
	for (int i = 0; i < vid.width * vid.height; ++i) // fog bias array
		randarr[i] = (lfsr_random() & 0xFFFF) / 65535.0f; // LFSR random number normalized to [0,1]
	fog_initialized = 1;
}

void R_DrawFog()
{
	if (!fog_initialized)
		R_InitFog();
	int fogstyle = 2;
	for (int y = 0; y < vid.height; ++y) { // fog! just a PoC, don't use pls.
	for (int x = 0; x < vid.width; ++x) {
		int i = x + y * vid.width;
		int bias = randarr[vid.width*vid.height - i] * 10;
		float fog_factor = compute_fog(d_pzbuffer[i] + bias, fog.value);
		if (fogstyle == 0) { // noisy
			float random_val = (lfsr_random() & 0xFFFF) / 65535.0f; // LFSR random number normalized to [0,1]
			if (random_val < fog_factor)
				((unsigned char *)(screen->pixels))[i] = 0; // replace with black, TODO colors
		}
		else if (fogstyle == 1) { // dither
			if (dither(x, y, fog_factor))
				((unsigned char *)(screen->pixels))[i] = 0;
		}
		else if (fogstyle == 2) { // dither + noise
			fog_factor += (randarr[i] - 0.5) * 0.2;
			fog_factor = fog_factor < 0.1 ? 0 : fog_factor;
			if (dither(x, y, fog_factor))
				((unsigned char *)(screen->pixels))[i] = 0;
		}
		else { // mix
			((unsigned char *)(screen->pixels))[i] = sw_avg(((unsigned char *)(screen->pixels))[i], fog_factor);
		}
	}
	}
}

