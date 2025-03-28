// Copyright (C) 1996-2001 Id Software, Inc.
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
	unsigned char *data;
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

msurface_t *r_skyfaces;
mplane_t r_skyplanes[6];
mtexinfo_t r_skytexinfo[6];
mvertex_t *r_skyverts;
medge_t *r_skyedges;
int *r_skysurfedges;

int skybox_planes[12] = {2,-128, 0,-128, 2,128, 1,128, 0,128, 1,-128};
int box_edges[24]={1,2, 2,3, 3,4, 4,1, 1,5, 5,6, 6,2, 7,8, 8,6, 5,7, 8,3, 7,4};
int box_faces[6] = {0,0,2,2,2,0};
int box_surfedges[24] = { 1,2,3,4, -1,5,6,7, 8,9,-6,10,
			-2,-7,-9,11, 12,-3,-11,-8, -12,-10,-5,-4};
vec3_t box_vecs[6][2] = { { {0,-1,0}, {-1,0,0} }, { {0,1,0}, {0,0,-1} },
			{ {0,-1,0}, {1,0,0} }, { {1,0,0}, {0,0,-1} },
			{ {0,-1,0}, {0,0,-1} }, { {-1,0,0}, {0,0,-1} } };
float box_verts[8][3] = { {-1,-1,-1}, {-1,1,-1}, {1,1,-1}, {1,-1,-1},
			 {-1,-1,1}, {-1,1,1}, {1,-1,1}, {1,1,1} };

byte *Image_LoadImage (const char *name, int *width, int *height);
unsigned char rgbtoi(unsigned char r, unsigned char g, unsigned char b);

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
			nonefound = false;
			int ww = skybox_textures[i].width;
			int hh = skybox_textures[i].height;
			skybox_textures[i].data = malloc(ww*hh); //TODO proper memory management
			unsigned char *pdest = skybox_textures[i].data;
			unsigned char *psrc = data;
			for (int j = 0; j < ww*hh; j++, psrc+=4)
				pdest[j] = rgbtoi(psrc[0], psrc[1], psrc[2]);
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

void R_InitSkyBox()
{
	extern model_t *loadmodel;
	r_skyfaces = loadmodel->surfaces + loadmodel->numsurfaces;
	loadmodel->numsurfaces += 6;
	r_skyverts = loadmodel->vertexes + loadmodel->numvertexes;
	loadmodel->numvertexes += 8;
	r_skyedges = loadmodel->edges + loadmodel->numedges;
	loadmodel->numedges += 12;
	r_skysurfedges = loadmodel->surfedges + loadmodel->numsurfedges;
	loadmodel->numsurfedges += 24;
	if (loadmodel->numsurfaces > MAX_MAP_FACES
			|| loadmodel->numvertexes > MAX_MAP_VERTS
			|| loadmodel->numedges > MAX_MAP_EDGES)
		Sys_Error ("InitSkyBox: map overflow");
	memset (r_skyfaces, 0, 6*sizeof(*r_skyfaces));
	for (int i=0 ; i<6 ; i++) {
		r_skyplanes[i].normal[skybox_planes[i*2]] = 1;
		r_skyplanes[i].dist = skybox_planes[i*2+1];
		VectorCopy (box_vecs[i][0], r_skytexinfo[i].vecs[0]);
		VectorCopy (box_vecs[i][1], r_skytexinfo[i].vecs[1]);
		r_skyfaces[i].plane = &r_skyplanes[i];
		r_skyfaces[i].numedges = 4;
		r_skyfaces[i].flags = box_faces[i] | SURF_DRAWSKY;
		r_skyfaces[i].firstedge = loadmodel->numsurfedges-24+i*4;
		r_skyfaces[i].texinfo = &r_skytexinfo[i];
		r_skyfaces[i].texturemins[0] = -128;
		r_skyfaces[i].texturemins[1] = -128;
		r_skyfaces[i].extents[0] = 256;
		r_skyfaces[i].extents[1] = 256;
	}
	for (int i=0 ; i<24 ; i++)
		if (box_surfedges[i] > 0)
			r_skysurfedges[i] = loadmodel->numedges-13 + box_surfedges[i];
		else
			r_skysurfedges[i] = - (loadmodel->numedges-13 + -box_surfedges[i]);

	for(int i=0 ; i<12 ; i++) {
		r_skyedges[i].v[0] = loadmodel->numvertexes-9+box_edges[i*2+0];
		r_skyedges[i].v[1] = loadmodel->numvertexes-9+box_edges[i*2+1];
		r_skyedges[i].cachededgeoffset = 0;
	}
}

void R_EmitSkyBox()
{
	for (int i = 0; i < 8; i++) // set the eight fake vertexes
		for (int j = 0; j < 3; j++)
			r_skyverts[i].position[j] = r_origin[j] + box_verts[i][j]*128;
	for (int i = 0; i < 6; i++) // set the six fake planes
		if (skybox_planes[i*2+1] > 0)
			r_skyplanes[i].dist = r_origin[skybox_planes[i*2]]+128;
		else
			r_skyplanes[i].dist = r_origin[skybox_planes[i*2]]-128;
	for (int i = 0; i < 6; i++) { // fix texture offseets
		r_skytexinfo[i].vecs[0][3] = -DotProduct (r_origin, r_skytexinfo[i].vecs[0]);
		r_skytexinfo[i].vecs[1][3] = -DotProduct (r_origin, r_skytexinfo[i].vecs[1]);
	}
	int oldkey = r_currentkey; // emit the six faces
	r_currentkey = 0x7ffffff0;
	for (int i = 0; i < 6; i++)
		R_RenderFace (r_skyfaces + i, 15);
	r_currentkey = oldkey; // bsp sorting order
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
