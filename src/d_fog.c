#include "quakedef.h"
#include "d_local.h"

#define FOG_LUT_LEVELS 32

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
extern cvar_t r_nofog;
extern unsigned int sb_updates; // if >= vid.numpages, no update needed
unsigned char color_mix_lut[256][256][FOG_LUT_LEVELS];
int fog_lut_built = 0;

unsigned char rgbtoi(unsigned char r, unsigned char g, unsigned char b)
{ // todo? lab color or really anything more accurate
	unsigned char besti = 0;
	int bestdist = 9999999;
	unsigned char *p = vid_curpal;
	for (int i = 0; i < 256; ++i) {
		int pr = p[0];
		int pg = p[1];
		int pb = p[2];
		int dr = r - pr;
		int dg = g - pg;
		int db = b - pb;
		int dist = (dr < 0 ? -dr : dr) +
			(dg < 0 ? -dg : dg) +
			(db < 0 ? -db : db);
		if (dist < bestdist) {
			bestdist = dist;
			besti = (unsigned char)i;
			if (dist == 0)
				break; // found an exact match, return early
		}
		p += 3;
	}
	return besti;
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
	fog_pal_index = rgbtoi(r*255.0f, g*255.0f, b*255.0f);
}

void Fog_ParseWorldspawn () // from Quakespasm
{ // called at map load
	char key[128], value[4096];
	fog_density = 0; //initially no fog
	fog_red = 0;
	fog_green = 0;
	fog_blue = 0;
	/*TODOold_density = DEFAULT_DENSITY;
	old_red = DEFAULT_GRAY;
	old_green = DEFAULT_GRAY;
	old_blue = DEFAULT_GRAY;
	fade_time = 0.0;
	fade_done = 0.0;*/
	const char *data = COM_Parse(cl.worldmodel->entities);
	if (!data || com_token[0] != '{')
		return; // error
	while (1) {
		if (!data)
			return; // error
		if (com_token[0] == '}')
			break; // end of worldspawn
		if (com_token[0] == '_')
			q_strlcpy(key, com_token + 1, sizeof(key));
		else
			q_strlcpy(key, com_token, sizeof(key));
		while (key[0] && key[strlen(key)-1] == ' ') // remove trailing spaces
			key[strlen(key)-1] = 0;
		data = COM_ParseEx(data, CPE_ALLOWTRUNC);
		if (!data)
			return; // error
		q_strlcpy(value, com_token, sizeof(value));
		if (!strcmp("fog", key))
			sscanf(value, "%f %f %f %f", &fog_density, &fog_red, &fog_green, &fog_blue);
	}
	fog_pal_index = rgbtoi(fog_red*255.0f, fog_green*255.0f, fog_blue*255.0f);
}

unsigned int lfsr_random() {
	lfsr ^= lfsr >> 7;
	lfsr ^= lfsr << 9;
	lfsr ^= lfsr >> 13;
	return lfsr;
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

void build_color_mix_lut()
{
    for (int c1 = 0; c1 < 256; c1++) {
        for (int c2 = 0; c2 < 256; c2++) {
            unsigned char r1 = vid_curpal[c1*3+0];
            unsigned char g1 = vid_curpal[c1*3+1];
            unsigned char b1 = vid_curpal[c1*3+2];
            unsigned char r2 = vid_curpal[c2*3+0];
            unsigned char g2 = vid_curpal[c2*3+1];
            unsigned char b2 = vid_curpal[c2*3+2];
            for (int level = 0; level < FOG_LUT_LEVELS; level++) {
                float factor = (float)level / (FOG_LUT_LEVELS-1);
                // learp each RGB component
                unsigned char r = (unsigned char)(r1 + factor * (r2 - r1));
                unsigned char g = (unsigned char)(g1 + factor * (g2 - g1));
                unsigned char b = (unsigned char)(b1 + factor * (b2 - b1));
                unsigned char mixed_index = rgbtoi(r, g, b);
                color_mix_lut[c1][c2][level] = mixed_index;
            }
        }
    }
    fog_lut_built = 1;
}

void R_InitFog()
{
	fog_pal_index = rgbtoi(fog_red*255.0f, fog_green*255.0f, fog_blue*255.0f);
	if (!randarr) {
		randarr = malloc(vid.width * vid.height * sizeof(float)); // TODO not optimal, use the zone
		for (int i = 0; i < vid.width * vid.height; ++i) // fog bias array
			randarr[i] = (lfsr_random() & 0xFFFF) / 65535.0f; // LFSR random number normalized to [0,1]
	}
	if (!fog_lut_built)
		build_color_mix_lut();
	fog_initialized = 1;
}

float compute_fog(int z) {
	const int fog_scale = 32;
	z = fog_scale > z ? fog_scale : z; // prevent distant objects from getting no fog
	z /= fog_scale;
	return expf(-(1.0f-fog_density) * (1.0f-fog_density) * (float)(z * z));
}

void R_DrawFog() {
	if (!fog_density || r_nofog.value)
		return;
	if (!fog_initialized)
		R_InitFog();
	sb_updates = 0; // draw sbar over fog
	int style = r_fogstyle.value;
	for (int y = 0; y < vid.height; ++y) {
	for (int x = 0; x < vid.width; ++x) {
		int i = x + y * vid.width;
		int bias = randarr[vid.width*vid.height - i] * 10;
		float fog_factor = compute_fog(d_pzbuffer[i] + bias);
		switch (style) {
			case 0: // noisy
				float random_val = (lfsr_random() & 0xFFFF) / 65535.0f;
				if (random_val < fog_factor)
					((unsigned char *)(screen->pixels))[i] = fog_pal_index;
				break;
			case 1: // dither
				if (dither(x, y, fog_factor))
					((unsigned char *)(screen->pixels))[i] = fog_pal_index;
				break;
			case 2: // dither + noise
				fog_factor += (randarr[i] - 0.5f) * 0.2f;
				fog_factor = fog_factor < 0.1f ? 0 : fog_factor;
				if (dither(x, y, fog_factor))
					((unsigned char *)(screen->pixels))[i] = fog_pal_index;
				break;
			default:
			case 3: // mix
				if (fog_factor < 1) 
					((unsigned char *)(screen->pixels))[i] =
						color_mix_lut[((unsigned char *)(screen->pixels))[i]]
							[fog_pal_index][(int)(fog_factor*FOG_LUT_LEVELS)];
				break;
		}
	}
	}
}
