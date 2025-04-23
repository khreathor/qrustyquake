// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

// r_surf.c: surface-related refresh code

#include "quakedef.h"
#include "r_local.h"

drawsurf_t r_drawsurf;

int lightleft, sourcesstep, blocksize, sourcetstep;
int lightleft_g;
int lightleft_b;
int lightdelta, lightdeltastep;
int lightright, lightleftstep, lightrightstep, blockdivshift;
int lightright_g, lightleftstep_g, lightrightstep_g;
int lightright_b, lightleftstep_b, lightrightstep_b;
unsigned blockdivmask;
void *prowdestbase;
unsigned char *pbasesource;
int surfrowbytes; // used by ASM files
unsigned *r_lightptr;
unsigned *r_lightptr_g;
unsigned *r_lightptr_b;
int r_stepback;
int r_lightwidth;
int r_numhblocks, r_numvblocks;
unsigned char *r_source, *r_sourcemax;
extern void init_color_conv();
extern cvar_t r_labmixpal;
extern cvar_t r_rgblighting;
extern unsigned char rgbtoi_lab(unsigned char r, unsigned char g, unsigned char b);
extern unsigned char rgbtoi(unsigned char r, unsigned char g, unsigned char b);

void R_DrawSurfaceBlock(int miplvl);

unsigned int blocklights[18 * 18];
unsigned int blocklights_g[18 * 18];
unsigned int blocklights_b[18 * 18];

void R_AddDynamicLights()
{
	msurface_t *surf = r_drawsurf.surf;
	int smax = (surf->extents[0] >> 4) + 1;
	int tmax = (surf->extents[1] >> 4) + 1;
	mtexinfo_t *tex = surf->texinfo;
	for (int lnum = 0; lnum < MAX_DLIGHTS; lnum++) {
		if (!(surf->dlightbits & (1 << lnum)))
			continue; // not lit by this light
		float rad = cl_dlights[lnum].radius;
		float dist = DotProduct(cl_dlights[lnum].origin,
				surf->plane->normal) - surf->plane->dist;
		rad -= fabs(dist);
		float minlight = cl_dlights[lnum].minlight;
		if (rad < minlight)
			continue;
		minlight = rad - minlight;
		vec3_t impact, local;
		for (int i = 0; i < 3; i++)
			impact[i] = cl_dlights[lnum].origin[i] -
				surf->plane->normal[i] * dist;
		local[0] = DotProduct(impact, tex->vecs[0]) + tex->vecs[0][3];
		local[1] = DotProduct(impact, tex->vecs[1]) + tex->vecs[1][3];
		local[0] -= surf->texturemins[0];
		local[1] -= surf->texturemins[1];
		for (int t = 0; t < tmax; t++) {
			int td = local[1] - t * 16;
			if (td < 0)
				td = -td;
			for (int s = 0; s < smax; s++) {
				int sd = local[0] - s * 16;
				if (sd < 0)
					sd = -sd;
				dist = sd > td ? sd + td / 2 : td + sd / 2;
				if (dist < minlight)
					blocklights[t*smax+s] += (rad-dist)*256;
			}
		}
	}
}


void R_BuildLightMap()
{ // Combine and scale multiple lightmaps into the 8.8 format in blocklights
	msurface_t *surf = r_drawsurf.surf;
	int smax = surf->extents[0] / 16 + 1;
	int tmax = surf->extents[1] / 16 + 1;
	int size = smax * tmax;
	byte *lightmap = surf->samples;
	if (r_fullbright.value || !cl.worldmodel->lightdata) {
		for (int i = 0; i < size; i++) {
			blocklights[i] = 0;
			blocklights_g[i] = 0;
			blocklights_b[i] = 0;
		}
		return;
	}
	for (int i = 0; i < size; i++) { // clear to ambient
		blocklights[i] = r_refdef.ambientlight << 8;
		blocklights_g[i] = r_refdef.ambientlight << 8;
		blocklights_b[i] = r_refdef.ambientlight << 8;
	}
	if (lightmap) // add all the lightmaps
		for (int maps = 0; maps < MAXLIGHTMAPS && surf->styles[maps] != 255; maps++) {
			unsigned int scale = r_drawsurf.lightadj[maps];	// 8.8 fraction         
			unsigned int *bl = blocklights;
			unsigned int *bl_g = blocklights_g;
			unsigned int *bl_b = blocklights_b;
			for (int i = 0; i < size; i++) {
				*bl++ += *lightmap++ * scale;
				*bl_g++ += *lightmap++ * scale;
				*bl_b++ += *lightmap++ * scale;
			}
		}
	if (surf->dlightframe == r_framecount) // add all the dynamic lights
		R_AddDynamicLights();
	for (int i = 0; i < size; i++) { // bound, invert, and shift
		int t = (255 * 256 - (int)blocklights[i]) >> (8 - VID_CBITS);
		if (t < 64)
			t = 64;
		blocklights[i] = t;
	}
	for (int i = 0; i < size; i++) { // bound, invert, and shift
		int t = (255 * 256 - (int)blocklights_g[i]) >> (8 - VID_CBITS);
		if (t < 64)
			t = 64;
		blocklights_g[i] = t;
	}
	for (int i = 0; i < size; i++) { // bound, invert, and shift
		int t = (255 * 256 - (int)blocklights_b[i]) >> (8 - VID_CBITS);
		if (t < 64)
			t = 64;
		blocklights_b[i] = t;
	}
}

texture_t *R_TextureAnimation(texture_t *base)
{ // Returns the proper texture for a given time and base texture
	if (currententity->frame && base->alternate_anims)
		base = base->alternate_anims;
	if (!base->anim_total)
		return base;
	int reletive = (int)(cl.time * 10) % base->anim_total;
	int count = 0;
	while (base->anim_min > reletive || base->anim_max <= reletive) {
		base = base->anim_next;
		if (!base)
			Sys_Error("R_TextureAnimation: broken cycle");
		if (++count > 100)
			Sys_Error("R_TextureAnimation: infinite cycle");
	}
	return base;
}

void R_DrawSurface()
{
	void (*pblockdrawer)();
	R_BuildLightMap(); // calculate the lightings
	surfrowbytes = r_drawsurf.rowbytes;
	texture_t *mt = r_drawsurf.texture;
	r_source = (byte *) mt + mt->offsets[r_drawsurf.surfmip];
	// the fractional light values should range from 0 to (VID_GRADES - 1)
	// << 16 from a source range of 0 - 255
	int texwidth = mt->width >> r_drawsurf.surfmip;
	blocksize = 16 >> r_drawsurf.surfmip;
	blockdivshift = 4 - r_drawsurf.surfmip;
	blockdivmask = (1 << blockdivshift) - 1;
	r_lightwidth = (r_drawsurf.surf->extents[0] >> 4) + 1;
	r_numhblocks = r_drawsurf.surfwidth >> blockdivshift;
	r_numvblocks = r_drawsurf.surfheight >> blockdivshift;
	// TODO: only needs to be set when there is a display settings change
	int horzblockstep = blocksize;
	int smax = mt->width >> r_drawsurf.surfmip;
	int twidth = texwidth;
	int tmax = mt->height >> r_drawsurf.surfmip;
	sourcetstep = texwidth;
	r_stepback = tmax * twidth;
	r_sourcemax = r_source + (tmax * smax);
	int soffset = r_drawsurf.surf->texturemins[0];
	int basetoffset = r_drawsurf.surf->texturemins[1];
	// << 16 components are to guarantee positive values for %
	soffset = ((soffset >> r_drawsurf.surfmip) + (smax << 16)) % smax;
	unsigned char *basetptr = &r_source[((((basetoffset>>r_drawsurf.surfmip)
				+ (tmax << 16)) % tmax) * twidth)];
	unsigned char *pcolumndest = r_drawsurf.surfdat;
	for (int u = 0; u < r_numhblocks; u++) {
		r_lightptr = blocklights + u;
		r_lightptr_g = blocklights_g + u;
		r_lightptr_b = blocklights_b + u;
		prowdestbase = pcolumndest;
		pbasesource = basetptr + soffset;
		R_DrawSurfaceBlock(r_drawsurf.surfmip);
		soffset = soffset + blocksize;
		if (soffset >= smax)
			soffset = 0;
		pcolumndest += horzblockstep;
	}
}

void R_DrawSurfaceBlock(int miplvl)
{
	unsigned char *psource = pbasesource;
	unsigned char *prowdest = prowdestbase;
	for (int v = 0; v < r_numvblocks; v++) {
		// FIXME: make these locals?
		// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0]; // Red
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		lightleftstep = (r_lightptr[0] - lightleft) >> (4-miplvl);
		lightrightstep = (r_lightptr[1] - lightright) >> (4-miplvl);
		lightleft_g = r_lightptr_g[0]; // Green
		lightright_g = r_lightptr_g[1];
		r_lightptr_g += r_lightwidth;
		lightleftstep_g = (r_lightptr_g[0] - lightleft_g) >> (4-miplvl);
		lightrightstep_g = (r_lightptr_g[1] - lightright_g) >> (4-miplvl);
		lightleft_b = r_lightptr_b[0]; // Blue
		lightright_b = r_lightptr_b[1];
		r_lightptr_b += r_lightwidth;
		lightleftstep_b = (r_lightptr_b[0] - lightleft_b) >> (4-miplvl);
		lightrightstep_b = (r_lightptr_b[1] - lightright_b) >> (4-miplvl);
		for (int i = 0; i < 1 << (4-miplvl); i++) {
			int lighttemp = lightleft - lightright; // Red
			int lightstep = lighttemp >> (4-miplvl);
			int light = lightright;
			int lighttemp_g = lightleft_g - lightright_g; // Green
			int lightstep_g = lighttemp_g >> (4-miplvl);
			int light_g = lightright_g;
			int lighttemp_b = lightleft_b - lightright_b; // Blue
			int lightstep_b = lighttemp_b >> (4-miplvl);
			int light_b = lightright_b;
			for (int b = (1 << (4-miplvl)) - 1; b >= 0; b--) {
				if (r_rgblighting.value == 0)
				{
					unsigned char pix = psource[b];
					prowdest[b] = ((unsigned char *)vid.colormap)
					    [(light & 0xFF00) + pix];
					light += lightstep;
				}
				else {
					unsigned char tex = psource[b];
					if (r_labmixpal.value == 1) {
						init_color_conv();
						prowdest[b] = rgbtoi_lab(
							vid_curpal[((unsigned char*)vid.colormap)[(light&0xFF00)+tex]*3+0],
							vid_curpal[((unsigned char*)vid.colormap)[(light_g&0xFF00)+tex]*3+1],
							vid_curpal[((unsigned char*)vid.colormap)[(light_b&0xFF00)+tex]*3+2]);
					}
					else
						prowdest[b] = rgbtoi(
							vid_curpal[((unsigned char*)vid.colormap)[(light&0xFF00)+tex]*3+0],
							vid_curpal[((unsigned char*)vid.colormap)[(light_g&0xFF00)+tex]*3+1],
							vid_curpal[((unsigned char*)vid.colormap)[(light_b&0xFF00)+tex]*3+2]);
					light += lightstep;
					light_g += lightstep_g;
					light_b += lightstep_b;
				}
			}
			psource += sourcetstep;
			lightright += lightrightstep;
			lightleft += lightleftstep;
			prowdest += surfrowbytes;
		}
		if (psource >= r_sourcemax)
			psource -= r_stepback;
	}
}

void R_GenTurbTile(pixel_t *pbasetex, void *pdest)
{
	int *turb = sintable + ((int)(cl.time * SPEED) & (CYCLE - 1));
	byte *pd = (byte *) pdest;
	for (int i = 0; i < TILE_SIZE; i++) {
		for (int j = 0; j < TILE_SIZE; j++) {
			int s = (((j << 16) + turb[i & (CYCLE-1)]) >> 16) & 63;
			int t = (((i << 16) + turb[j & (CYCLE-1)]) >> 16) & 63;
			*pd++ = *(pbasetex + (t << 6) + s);
		}
	}
}
