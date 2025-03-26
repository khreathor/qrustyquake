// Copyright (C) 1996-1997 Id Software, Inc.
// Copyright (C) 2002-2009 John Fitzgibbons and others
// Copyright (C) 2007-2008 Kristian Duske
// Copyright (C) 2010-2014 QuakeSpasm developers
// GPLv3 See LICENSE for details.

#include "quakedef.h"
#include "r_local.h"
#include "d_local.h"

typedef struct cubemap_s
{
	int width, height;
	char *data;
} cubemap_t;

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
char skybox_name[1024]; // name of current skybox, or "" if no skybox
static const char *suf[6] = {"rt", "bk", "lf", "ft", "up", "dn"};
cubemap_t skybox_textures[6];

byte *Image_LoadImage (const char *name, int *width, int *height);

void Sky_LoadSkyBox (const char *name)
{
	int i, mark, width, height;
	char filename[MAX_OSPATH];
	byte *data;
	qboolean nonefound = true;
	if (strcmp(skybox_name, name) == 0) //TODO
		return; // no change
	for (i=0; i<6; i++) { // purge old textures
		//TODOif (skybox_textures[i])
			//TODOTexMgr_FreeTexture (skybox_textures[i]);
		skybox_textures[i].data = NULL;
	}
	if (name[0] == 0) { // turn off skybox if sky is set to ""
		skybox_name[0] = 0;
		return;
	}
	for (i=0; i<6; i++) { // load textures
		mark = Hunk_LowMark ();
		q_snprintf (filename, sizeof(filename), "gfx/env/%s%s", name, suf[i]);
		data = Image_LoadImage (filename, &skybox_textures[i].width, &skybox_textures[i].height);
		if (data) {
			skybox_textures[i].data = data;
			nonefound = false;
			__asm__("int3");
		}
		else {
			Con_Printf ("Couldn't load %s\n", filename);
			//TODOskybox_textures[i] = notexture;
		}
		Hunk_FreeToLowMark (mark);
	}
	if (nonefound) {// go back to scrolling sky if skybox is totally missing
		for (i=0; i<6; i++) {
			//TODOif (skybox_textures[i] && skybox_textures[i] != notexture)
			//TODO	TexMgr_FreeTexture (skybox_textures[i]);
			skybox_textures[i].data = NULL;
		}
		skybox_name[0] = 0;
		return;
	}
	q_strlcpy(skybox_name, name, sizeof(skybox_name));
	r_skysource = skybox_textures[0].data; //placeholder
}

void Sky_SkyCommand_f()
{
	switch (Cmd_Argc()) {
		case 1:
			Con_Printf("\"sky\" is \"%s\"\n", skybox_name);
			break;
		case 2:
			Sky_LoadSkyBox(Cmd_Argv(1));
			break;
		default:
			Con_Printf("usage: sky <skyname>\n");
	}
}

void Sky_Init()
{
	Cmd_AddCommand ("sky",Sky_SkyCommand_f);
        skybox_name[0] = 0;
        for (int i = 0; i < 6; i++)
                skybox_textures[i].data = NULL;
}

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
