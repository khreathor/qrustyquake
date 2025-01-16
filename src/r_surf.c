// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

// r_surf.c: surface-related refresh code

#include "quakedef.h"
#include "r_local.h"

drawsurf_t r_drawsurf;

int lightleft, sourcesstep, blocksize, sourcetstep;
int lightdelta, lightdeltastep;
int lightright, lightleftstep, lightrightstep, blockdivshift;
unsigned blockdivmask;
void *prowdestbase;
unsigned char *pbasesource;
int surfrowbytes; // used by ASM files
unsigned *r_lightptr;
int r_stepback;
int r_lightwidth;
int r_numhblocks, r_numvblocks;
unsigned char *r_source, *r_sourcemax;

void R_DrawSurfaceBlock8_mip0();
void R_DrawSurfaceBlock8_mip1();
void R_DrawSurfaceBlock8_mip2();
void R_DrawSurfaceBlock8_mip3();

static void (*surfmiptable[4])() = {
	R_DrawSurfaceBlock8_mip0,
	R_DrawSurfaceBlock8_mip1,
	R_DrawSurfaceBlock8_mip2,
	R_DrawSurfaceBlock8_mip3
};

unsigned int blocklights[18 * 18];

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
		for (int i = 0; i < size; i++)
			blocklights[i] = 0;
		return;
	}
	for (int i = 0; i < size; i++) // clear to ambient
		blocklights[i] = r_refdef.ambientlight << 8;
	if (lightmap) // add all the lightmaps
		for (int maps = 0; maps < MAXLIGHTMAPS && surf->styles[maps] != 255;
		     maps++) {
			unsigned int scale = r_drawsurf.lightadj[maps];	// 8.8 fraction         
			for (int i = 0; i < size; i++)
				blocklights[i] += lightmap[i] * scale;
			lightmap += size; // skip to next lightmap
		}
	if (surf->dlightframe == r_framecount) // add all the dynamic lights
		R_AddDynamicLights();
	for (int i = 0; i < size; i++) { // bound, invert, and shift
		int t = (255 * 256 - (int)blocklights[i]) >> (8 - VID_CBITS);
		if (t < 64)
			t = 64;
		blocklights[i] = t;
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
	pblockdrawer = surfmiptable[r_drawsurf.surfmip];
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
		prowdestbase = pcolumndest;
		pbasesource = basetptr + soffset;
		(*pblockdrawer) ();
		soffset = soffset + blocksize;
		if (soffset >= smax)
			soffset = 0;
		pcolumndest += horzblockstep;
	}
}

// CyanBun96: TODO combine these
void R_DrawSurfaceBlock8_mip0()
{
	unsigned char *psource = pbasesource;
	unsigned char *prowdest = prowdestbase;
	for (int v = 0; v < r_numvblocks; v++) {
		// FIXME: make these locals?
		// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		lightleftstep = (r_lightptr[0] - lightleft) >> 4;
		lightrightstep = (r_lightptr[1] - lightright) >> 4;
		for (int i = 0; i < 16; i++) {
			int lighttemp = lightleft - lightright;
			int lightstep = lighttemp >> 4;
			int light = lightright;
			for (int b = 15; b >= 0; b--) {
				unsigned char pix = psource[b];
				prowdest[b] = ((unsigned char *)vid.colormap)
						[(light & 0xFF00) + pix];
				light += lightstep;
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

void R_DrawSurfaceBlock8_mip1()
{
	unsigned char *psource = pbasesource;
	unsigned char *prowdest = prowdestbase;
	for (int v = 0; v < r_numvblocks; v++) {
		// FIXME: make these locals?
		// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		lightleftstep = (r_lightptr[0] - lightleft) >> 3;
		lightrightstep = (r_lightptr[1] - lightright) >> 3;
		for (int i = 0; i < 8; i++) {
			int lighttemp = lightleft - lightright;
			int lightstep = lighttemp >> 3;
			int light = lightright;
			for (int b = 7; b >= 0; b--) {
				unsigned char pix = psource[b];
				prowdest[b] = ((unsigned char *)vid.colormap)
						[(light & 0xFF00) + pix];
				light += lightstep;
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

void R_DrawSurfaceBlock8_mip2()
{
	unsigned char *psource = pbasesource;
	unsigned char *prowdest = prowdestbase;
	for (int v = 0; v < r_numvblocks; v++) {
		// FIXME: make these locals?
		// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		lightleftstep = (r_lightptr[0] - lightleft) >> 2;
		lightrightstep = (r_lightptr[1] - lightright) >> 2;
		for (int i = 0; i < 4; i++) {
			int lighttemp = lightleft - lightright;
			int lightstep = lighttemp >> 2;
			int light = lightright;
			for (int b = 3; b >= 0; b--) {
				unsigned char pix = psource[b];
				prowdest[b] = ((unsigned char *)vid.colormap)
						[(light & 0xFF00) + pix];
				light += lightstep;
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

void R_DrawSurfaceBlock8_mip3()
{
	unsigned char *psource = pbasesource;
	unsigned char *prowdest = prowdestbase;
	for (int v = 0; v < r_numvblocks; v++) {
		// FIXME: make these locals?
		// FIXME: use delta rather than both right and left, like ASM?
		lightleft = r_lightptr[0];
		lightright = r_lightptr[1];
		r_lightptr += r_lightwidth;
		lightleftstep = (r_lightptr[0] - lightleft) >> 1;
		lightrightstep = (r_lightptr[1] - lightright) >> 1;
		for (int i = 0; i < 2; i++) {
			int lighttemp = lightleft - lightright;
			int lightstep = lighttemp >> 1;
			int light = lightright;
			for (int b = 1; b >= 0; b--) {
				unsigned char pix = psource[b];
				prowdest[b] = ((unsigned char *)vid.colormap)
				    [(light & 0xFF00) + pix];
				light += lightstep;
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
