#include "quakedef.h"
#include "d_local.h"
#include "r_local.h"

typedef struct {
	float l, a, b;
} lab;

extern cvar_t r_labmixpal;

unsigned char color_mix_lut[256][256][FOG_LUT_LEVELS];
int fog_lut_built = 0;
float gamma_lut[256];
int color_conv_initialized = 0;
lab lab_palette[256];
unsigned char lit_lut[LIT_LUT_RES*LIT_LUT_RES*LIT_LUT_RES];
int lit_lut_initialized = 0;

static inline float lab_f(float t)
{ // cube-root if > epsilon, linear otherwise
	const float epsilon = 0.008856f;
	const float kappa = 903.3f;
	return (t > epsilon) ? cbrtf(t) : ((kappa * t + 16.0f) / 116.0f);
}

static inline lab rgb_lin_to_lab(float r, float g, float b)
{
	float x = r * 0.4124f + g * 0.3576f + b * 0.1805f;
	float y = r * 0.2126f + g * 0.7152f + b * 0.0722f;
	float z = r * 0.0193f + g * 0.1192f + b * 0.9505f;
	x /= 0.95047f; // normalize by d65 white
	z /= 1.08883f;
	float fx = lab_f(x);
	float fy = lab_f(y);
	float fz = lab_f(z);
	lab lab = {116.0f * fy - 16.0f, 500.0f * (fx - fy), 200.0f * (fy - fz)};
	return lab;
}

void init_color_conv()
{
	if (color_conv_initialized) return;
	color_conv_initialized = 1;
	for (int i = 0; i < 256; ++i) {
		float v = i / 255.0f;
		gamma_lut[i] = (v > 0.04045f) ?
			powf((v + 0.055f) / 1.055f, 2.4f) : (v / 12.92f);
	}
	for (int i = 0, p = 0; i < 256; ++i, p += 3) {
		float lr = gamma_lut[host_basepal[p + 0]];
		float lg = gamma_lut[host_basepal[p + 1]];
		float lb = gamma_lut[host_basepal[p + 2]];
		lab_palette[i] = rgb_lin_to_lab(lr, lg, lb);
	}
}

unsigned char rgbtoi_lab(unsigned char r, unsigned char g, unsigned char b)
{
	lab lab0 = rgb_lin_to_lab(gamma_lut[r], gamma_lut[g], gamma_lut[b]);
	float bestdist = FLT_MAX;
	unsigned char besti = 0;
	for (int i = 0; i < 256; ++i) {
		float dl = lab0.l - lab_palette[i].l;
		float da = lab0.a - lab_palette[i].a;
		float db = lab0.b - lab_palette[i].b;
		float dist = dl * dl + da * da + db * db;
		if (dist < bestdist) {
			bestdist = dist;
			besti = (unsigned char)i;
			if (dist == 0.0f) break;
		}
	}
	return besti;
}

unsigned char rgbtoi(unsigned char r, unsigned char g, unsigned char b)
{
	unsigned char besti = 0;
	int bestdist = 0x7fffffff;
	unsigned char *p = host_basepal;
	for (int i = 0; i < 256; ++i, p += 3) {
		int dr = r - p[0]; // squared euclidean distance
		int dg = g - p[1];
		int db = b - p[2];
		int dist = dr * dr + dg * dg + db * db;
		if (dist < bestdist) {
			bestdist = dist;
			besti = (unsigned char)i;
			if (dist == 0) // exact match, return early
				return besti;
		}
	}
	return besti;
}

void build_color_mix_lut(cvar_t *cvar)
{
	unsigned char (*convfunc)(unsigned char, unsigned char, unsigned char);
	if (r_labmixpal.value == 1) {
		init_color_conv();
		convfunc = rgbtoi_lab;
	}
	else
		convfunc = rgbtoi;
	for (int c1 = 0; c1 < 256; c1++) {
		for (int c2 = 0; c2 < 256; c2++) {
			unsigned char r1 = host_basepal[c1*3+0];
			unsigned char g1 = host_basepal[c1*3+1];
			unsigned char b1 = host_basepal[c1*3+2];
			unsigned char r2 = host_basepal[c2*3+0];
			unsigned char g2 = host_basepal[c2*3+1];
			unsigned char b2 = host_basepal[c2*3+2];
			for (int level = 0; level < FOG_LUT_LEVELS; level++) {
				float factor = (float)level / (FOG_LUT_LEVELS-1);
				// lerp each RGB component
				unsigned char r = (unsigned char)(r1 + factor * (r2 - r1));
				unsigned char g = (unsigned char)(g1 + factor * (g2 - g1));
				unsigned char b = (unsigned char)(b1 + factor * (b2 - b1));
				unsigned char mixed_index = convfunc(r, g, b);
				color_mix_lut[c1][c2][level] = mixed_index;
			}
		}
	}
	fog_lut_built = 1;
}

void R_BuildLitLUT()
{
	unsigned char (*convfunc)(unsigned char, unsigned char, unsigned char);
	if (r_labmixpal.value == 1) {
		init_color_conv();
		convfunc = rgbtoi_lab;
	}
	else
		convfunc = rgbtoi;
	const int llr = LIT_LUT_RES;
	for (int r_ = 0; r_ < llr; ++r_) {
	for (int g_ = 0; g_ < llr; ++g_) {
	for (int b_ = 0; b_ < llr; ++b_) {
		int rr = (r_ * 255) / (llr - 1);
		int gg = (g_ * 255) / (llr - 1);
		int bb = (b_ * 255) / (llr - 1);
		lit_lut[r_+g_*llr+b_*llr*llr] = convfunc(rr, gg, bb);
	} } }
	lit_lut_initialized = 1;
}

