// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

#include "quakedef.h"
#include "r_local.h"
#include "d_local.h"

int iskyspeed = 8;
int iskyspeed2 = 2;
float skyspeed, skyspeed2;
float skytime;
byte *r_skysource;
int r_skymade;
byte bottomsky[128 * 131];
byte bottommask[128 * 131];
byte newsky[128 * 256];	
// newsky and topsky both pack in here, 128 bytes of newsky on the left of each
// scan, 128 bytes of topsky on the right, because the low-level drawers need
// 256-byte scan widths

void R_InitSky(texture_t *mt)
{ // A sky texture is 256*128, with the right side being a masked overlay
	byte *src = (byte *) mt + mt->offsets[0];
	for (int i = 0; i < 128; i++)
		for (int j = 0; j < 128; j++)
			newsky[(i * 256) + j + 128] = src[i * 256 + j + 128];
	for (int i = 0; i < 128; i++)
		for (int j = 0; j < 131; j++) {
			if (src[i * 256 + (j & 0x7F)]) {
				bottomsky[(i * 131) + j] =
				    src[i * 256 + (j & 0x7F)];
				bottommask[(i * 131) + j] = 0;
			} else {
				bottomsky[(i * 131) + j] = 0;
				bottommask[(i * 131) + j] = 0xff;
			}
		}
	r_skysource = newsky;
}

void R_MakeSky()
{
	static int xlast = -1, ylast = -1;
	int xshift = skytime * skyspeed;
	int yshift = skytime * skyspeed;
	if ((xshift == xlast) && (yshift == ylast))
		return;
	xlast = xshift;
	ylast = yshift;
	unsigned int *pnewsky = (unsigned int *)&newsky[0];
	for (int y = 0; y < SKYSIZE; y++) {
		int baseofs = ((y + yshift) & SKYMASK) * 131;
		for (int x = 0; x < SKYSIZE; x++) {
			int ofs = baseofs + ((x + xshift) & SKYMASK);
			*(byte *) pnewsky = (*((byte *) pnewsky + 128) &
					*(byte *) & bottommask[ofs]) |
					*(byte *) & bottomsky[ofs];
			pnewsky = (unsigned *)((byte *) pnewsky + 1);
		}
		pnewsky += 128 / sizeof(unsigned);
	}
	r_skymade = 1;
}

void R_GenSkyTile(void *pdest)
{
	int xshift = skytime * skyspeed;
	int yshift = skytime * skyspeed;
	unsigned int *pnewsky = (unsigned int *)&newsky[0];
	unsigned int *pd = (unsigned int *)pdest;
	for (int y = 0; y < SKYSIZE; y++) {
		int baseofs = ((y + yshift) & SKYMASK) * 131;
		for (int x = 0; x < SKYSIZE; x++) {
			int ofs = baseofs + ((x + xshift) & SKYMASK);
			*(byte *) pd = (*((byte *) pnewsky + 128) &
					*(byte *) & bottommask[ofs]) |
			    *(byte *) & bottomsky[ofs];
			pnewsky = (unsigned int *)((byte *) pnewsky + 1);
			pd = (unsigned int *)((byte *) pd + 1);
		}
		pnewsky += 128 / sizeof(unsigned int);
	}
}

void R_GenSkyTile16(void *pdest)
{
	int xshift = skytime * skyspeed;
	int yshift = skytime * skyspeed;
	byte *pnewsky = (byte *) & newsky[0];
	unsigned short *pd = (unsigned short *)pdest;
	for (int y = 0; y < SKYSIZE; y++) {
		int baseofs = ((y + yshift) & SKYMASK) * 131;
		for (int x = 0; x < SKYSIZE; x++) {
			int ofs = baseofs + ((x + xshift) & SKYMASK);
			*pd = d_8to16table[(*(pnewsky + 128) &
					*(byte *) & bottommask[ofs]) |
					*(byte *) & bottomsky[ofs]];
			pnewsky++;
			pd++;
		}
		pnewsky += TILE_SIZE;
	}
}

void R_SetSkyFrame()
{
	skyspeed = iskyspeed;
	skyspeed2 = iskyspeed2;
	int g = GreatestCommonDivisor(iskyspeed, iskyspeed2);
	int s1 = iskyspeed / g;
	int s2 = iskyspeed2 / g;
	float temp = SKYSIZE * s1 * s2;
	skytime = cl.time - ((int)(cl.time / temp) * temp);
	r_skymade = 0;
}
