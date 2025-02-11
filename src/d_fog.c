#include "quakedef.h"
#include "d_local.h"

float *randarr; // used for noise bias
int fog_initialized = 0;
static unsigned int lfsr = 0x1337; // non-zero seed
float fog_density;
static float fog_red; // CyanBun96: we store the actual RGB values in these,
static float fog_green; // but they get quantized to a single index in the color
static float fog_blue; // palette before use, stored in fog_pal_index
unsigned char fog_pal_index;
extern unsigned char vid_curpal[256 * 3]; // RGB palette
extern cvar_t r_fogstyle;

unsigned char rgb_to_index(float inr, float ing, float inb)
{
	unsigned char r = inr * 255;
	unsigned char g = ing * 255;
	unsigned char b = inb * 255;
	unsigned char reti = 0;
	int mindist = 99999;
	unsigned char *pal = vid_curpal;
	for (int i = 0; i < 256; ++i) { // a very naive algo, replace with
		int dist = 0; // something actually color-theory based
		dist += r > pal[i*3+0] ? r - pal[i*3+0] : pal[i*3+0] - r;
		dist += g > pal[i*3+1] ? g - pal[i*3+1] : pal[i*3+1] - g;
		dist += b > pal[i*3+2] ? b - pal[i*3+2] : pal[i*3+2] - b;
		if (mindist > dist) {
			mindist = dist;
			reti = i;
		}
	}
	return reti;
}

void Fog_FogCommand_f () // yanked from Quakespasm, mostly
{ // TODO time transitions
	float d, r, g, b, t;
	switch (Cmd_Argc()) {
		default:
		case 1:
			Con_Printf("usage:\n");
			Con_Printf("   fog <density>\n");
			Con_Printf("   fog <red> <green> <blue>\n");
			Con_Printf("   fog <density> <red> <green> <blue>\n");
			Con_Printf("current values:\n");
			Con_Printf("   \"density\" is \"%f\"\n", fog_density);
			Con_Printf("   \"red\" is \"%f\"\n", fog_red);
			Con_Printf("   \"green\" is \"%f\"\n", fog_green);
			Con_Printf("   \"blue\" is \"%f\"\n", fog_blue);
			return;
		case 2:
			d = Q_atof(Cmd_Argv(1));
			t = 0.0f;
			r = fog_red;
			g = fog_green;
			b = fog_blue;
			break;
		case 3: //TEST
			d = Q_atof(Cmd_Argv(1));
			t = Q_atof(Cmd_Argv(2));
			r = fog_red;
			g = fog_green;
			b = fog_blue;
			break;
		case 4:
			d = fog_density;
			t = 0.0f;
			r = Q_atof(Cmd_Argv(1));
			g = Q_atof(Cmd_Argv(2));
			b = Q_atof(Cmd_Argv(3));
			break;
		case 5:
			d = Q_atof(Cmd_Argv(1));
			r = Q_atof(Cmd_Argv(2));
			g = Q_atof(Cmd_Argv(3));
			b = Q_atof(Cmd_Argv(4));
			t = 0.0f;
			break;
		case 6: //TEST
			d = Q_atof(Cmd_Argv(1));
			r = Q_atof(Cmd_Argv(2));
			g = Q_atof(Cmd_Argv(3));
			b = Q_atof(Cmd_Argv(4));
			t = Q_atof(Cmd_Argv(5));
			break;
	}
	if      (d < 0.0f) d = 0.0f;
	if      (r < 0.0f) r = 0.0f;
	else if (r > 1.0f) r = 1.0f;
	if      (g < 0.0f) g = 0.0f;
	else if (g > 1.0f) g = 1.0f;
	if      (b < 0.0f) b = 0.0f;
	else if (b > 1.0f) b = 1.0f;
	fog_density = d;
	fog_red = r;
	fog_green = g;
	fog_blue = b;
	fog_pal_index = rgb_to_index(r, g, b);
}

unsigned int lfsr_random() {
	lfsr ^= lfsr >> 7;
	lfsr ^= lfsr << 9;
	lfsr ^= lfsr >> 13;
	return lfsr;
}

float compute_fog(int z) {
	z /= 10; // TODO adjust
	return expf(-(1.0f-fog_density) * (1.0f-fog_density) * (float)(z * z));
}

unsigned char sw_avg(unsigned char v1, float f) {
	// the incredibly slow color version
	float scr_red = (float)vid_curpal[v1*3 + 0] / 256.0f;
	float scr_green = (float)vid_curpal[v1*3 + 1] / 256.0f;
	float scr_blue = (float)vid_curpal[v1*3 + 2] / 256.0f;
	float new_red = fog_red * f + scr_red * (1.0f - f);
	float new_green = fog_green * f + scr_green * (1.0f - f);
	float new_blue = fog_blue * f + scr_blue * (1.0f - f);
	return rgb_to_index(new_red, new_green, new_blue);
	/* the efficient but monochrome version
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
	*/
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
	for (int y = 0; y < vid.height; ++y) {
	for (int x = 0; x < vid.width; ++x) {
		int i = x + y * vid.width;
		int bias = randarr[vid.width*vid.height - i] * 10;
		float fog_factor = compute_fog(d_pzbuffer[i] + bias);
		if (r_fogstyle.value == 0) { // noisy
			float random_val = (lfsr_random() & 0xFFFF) / 65535.0f; // LFSR random number normalized to [0,1]
			if (random_val < fog_factor)
				((unsigned char *)(screen->pixels))[i] = fog_pal_index;
		}
		else if (r_fogstyle.value == 1) { // dither
			if (dither(x, y, fog_factor))
				((unsigned char *)(screen->pixels))[i] = fog_pal_index;
		}
		else if (r_fogstyle.value == 2) { // dither + noise
			fog_factor += (randarr[i] - 0.5) * 0.2;
			fog_factor = fog_factor < 0.1 ? 0 : fog_factor;
			if (dither(x, y, fog_factor))
				((unsigned char *)(screen->pixels))[i] = fog_pal_index;
		}
		else { // mix
			if (fog_factor < 1)
				((unsigned char *)(screen->pixels))[i] = sw_avg(((unsigned char *)(screen->pixels))[i], fog_factor);
		}
	}
	}
}

