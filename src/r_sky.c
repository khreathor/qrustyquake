// Copyright (C) 1996-2001 Id Software, Inc.
// Copyright (C) 2002-2009 John Fitzgibbons and others
// Copyright (C) 2007-2008 Kristian Duske
// Copyright (C) 2010-2014 QuakeSpasm developers
// GPLv3 See LICENSE for details.

#include "quakedef.h"

extern byte *Image_LoadImage (const char *name, int *width, int *height);

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

// Manoel Kasimier - skyboxes - begin
// Code taken from the ToChriS engine - Author: Vic
int                             r_skyframe;
msurface_t              *r_skyfaces;
mplane_t                r_skyplanes[6]; // Manoel Kasimier - edited
mtexinfo_t              r_skytexinfo[6];
byte         r_skypixels[6][SKYBOX_MAX_SIZE*SKYBOX_MAX_SIZE]; // Manoel Kasimier - edited
mvertex_t               *r_skyverts;
medge_t                 *r_skyedges;
int                             *r_skysurfedges;

// I just copied this data from a box map...
int skybox_planes[12] = {2,-128, 0,-128, 2,128, 1,128, 0,128, 1,-128};

int box_surfedges[24] = { 1,2,3,4,  -1,5,6,7,  8,9,-6,10,  -2,-7,-9,11,
  12,-3,-11,-8,  -12,-10,-5,-4};
int box_edges[24] = { 1,2, 2,3, 3,4, 4,1, 1,5, 5,6, 6,2, 7,8, 8,6, 5,7, 8,3, 7,4};

int     box_faces[6] = {0,0,2,2,2,0};

vec3_t  box_vecs[6][2] = {
        {       {0,-1,0}, {-1,0,0} }, { {0,1,0}, {0,0,-1} },
        {       {0,-1,0}, {1,0,0} }, { {1,0,0}, {0,0,-1} },
        { {0,-1,0}, {0,0,-1} }, { {-1,0,0}, {0,0,-1} }
};

// Manoel Kasimier - hi-res skyboxes - begin
vec3_t  box_bigvecs[6][2] = {
        {       {0,-2,0}, {-2,0,0} }, { {0,2,0}, {0,0,-2} },
        {       {0,-2,0}, {2,0,0} }, { {2,0,0}, {0,0,-2} },
        { {0,-2,0}, {0,0,-2} }, { {-2,0,0}, {0,0,-2} }
};
vec3_t  box_bigbigvecs[6][2] = {
        {       {0,-4,0}, {-4,0,0} }, { {0,4,0}, {0,0,-4} },
        {       {0,-4,0}, {4,0,0} }, { {4,0,0}, {0,0,-4} },
        { {0,-4,0}, {0,0,-4} }, { {-4,0,0}, {0,0,-4} }
}; // Manoel Kasimier - hi-res skyboxes - end
float   box_verts[8][3] = { {-1,-1,-1}, {-1,1,-1}, {1,1,-1}, {1,-1,-1},
				{-1,-1,1}, {-1,1,1}, {1,-1,1}, {1,1,1} };
// Manoel Kasimier - skyboxes - end

byte    *skyunderlay, *skyoverlay; // Manoel Kasimier - smooth sky
byte    bottomalpha[128*131]; // Manoel Kasimier - translucent sky

// Manoel Kasimier - skyboxes - begin
// Code taken from the ToChriS engine - Author: Vic (vic@quakesrc.org) (http://hkitchen.quakesrc.org/)
extern  mtexinfo_t              r_skytexinfo[6];
//extern        cbool           r_drawskybox;
byte                                    r_skypixels[6][SKYBOX_MAX_SIZE*SKYBOX_MAX_SIZE]; // Manoel Kasimier - edited
texture_t                               r_skytextures[6];
char                                    last_skybox_name[1024];

void Sky_LoadTexture (texture_t *mt)
{
	// Baker: Warn.
	if (mt->width != 256 || mt->height != 128) // Leave this.
		Con_Printf ("Standard sky texture %s expected to be 256 x 128 but is %d by %d ", mt->name, mt->width, mt->height);

	skyoverlay = (byte *)mt + mt->offsets[0]; // Manoel Kasimier - smooth sky
	skyunderlay = skyoverlay+128; // Manoel Kasimier - smooth sky
}

int R_LoadSkybox (const char *name);

void Sky_LoadSkyBox (const char *name)
{
	if (strcmp (skybox_name, name) == 0)
		return; //no change
	if (name[0] == 0) { // turn off skybox if sky is set to ""
		skybox_name[0] = 0;
		return;
	}
	// Baker: If name matches, we already have the pixels loaded and we don't
	// actually need to reload
	if (strcmp (name, last_skybox_name)) {
		if (cl.worldmodel && !R_LoadSkybox (name)) {
			skybox_name[0] = 0;
			return;
		}
	}
	q_strlcpy(skybox_name, name, sizeof(skybox_name));
	q_strlcpy(last_skybox_name, skybox_name, sizeof(last_skybox_name));
}

void R_BuildRGBLUT() {
	for (int r = 0; r < 256; r += 32) {
		for (int g = 0; g < 256; g += 32) {
			for (int b = 0; b < 256; b += 32) {
				int i = ((r/32) << 6) | ((g/32) << 3) | (b/32);
				rgb_lut[i] = rgbtoi(r, g, b);
			}
		}
	}
	rgb_lut_built = 1;
}

unsigned char fast_rgbtoi(unsigned char r, unsigned char g, unsigned char b)
{
    int i = ((r / 32) << 6) | ((g / 32) << 3) | (b / 32);
    return rgb_lut[i];
}

byte *Upscale_NearestNeighbor(byte *src, int width, int height, int *new_width,
	int *new_height) {
	*new_width = (width < 256) ? 256 : width;
	*new_height = (height < 256) ? 256 : height;
	if (*new_width == width && *new_height == height)
		return src;
	int scale_x = *new_width / width;
	int scale_y = *new_height / height;
	byte *dest = malloc((*new_width) * (*new_height));
	if (!dest) return 0;
	for (int y = 0; y < *new_height; ++y) {
		for (int x = 0; x < *new_width; ++x) {
			dest[y*(*new_width)+x] = src[y/scale_y*width+x/scale_x];
		}
	}
	return dest;
}

int R_LoadSkybox (const char *name)
{
	if (!name || !name[0]) {
		skybox_name[0] = 0;
		return false;
	}
	if (!strcmp (name, skybox_name)) // the same skybox we are using now
		return true;
	q_strlcpy (skybox_name, name, sizeof(skybox_name));
	int mark = Hunk_LowMark ();
	for (int i = 0 ; i < 6 ; i++) {
		char pathname[1024];
		char *suf[6] = {"rt", "bk", "lf", "ft", "up", "dn"};
		int r_skysideimage[6] = {5, 2, 4, 1, 0, 3};
		q_snprintf (pathname, sizeof(pathname), "gfx/env/%s%s", name, suf[r_skysideimage[i]]);
		int width, height;
		byte *pic = Image_LoadImage (pathname, &width, &height);
		unsigned char *pdest = pic; // CyanBun96: palettize in place
		unsigned char *psrc = pic; // with some noise dithering
		if (!rgb_lut_built)
			R_BuildRGBLUT();
		for (int j = 0; j < width*height; j++, psrc+=4) {
			int r = psrc[0] + (lfsr_random() % 16) - 7;
			int g = psrc[1] + (lfsr_random() % 16) - 7;
			int b = psrc[2] + (lfsr_random() % 16) - 7;
			pdest[j] = fast_rgbtoi(CLAMP(0,r,255),
						CLAMP(0,g,255),
						CLAMP(0,b,255));
		}
		byte *final_pic = Upscale_NearestNeighbor(pic,
				width, height, &width, &height);
		byte *origpic = pic;
		pic = final_pic;
		if (!pic) {
			Con_Printf ("Couldn't load %s", pathname);
			return false;
		}
		switch (width) { // Manoel Kasimier - hi-res skyboxes - begin
			case 1024: // falls through
			case 512: // falls through
			case 256: // We're good!
				break;
			default: // We aren't good
				Con_Printf ("skybox width (%d) for %s must be 256, 512, 1024", width, pathname);
				pic = origpic;
				Hunk_FreeToLowMark (mark);
				if (final_pic != origpic)
					free(final_pic);
				return false;
		}
		switch (height) {
			case 1024: // falls through
			case 512: // falls through
			case 256: // We're good!
				break;
			default:
				Con_Printf ("skybox height (%d) for %s must be 256, 512, 1024", height, pathname);
				pic = origpic;
				Hunk_FreeToLowMark (mark);
				if (final_pic != origpic)
					free(final_pic);
				return false;
		}
		r_skytexinfo[i].texture = &r_skytextures[i];
		r_skytexinfo[i].texture->width = width;
		r_skytexinfo[i].texture->height = height;
		r_skytexinfo[i].texture->offsets[0] = i;
		extern vec3_t box_vecs[6][2];
		extern vec3_t box_bigvecs[6][2];
		extern vec3_t box_bigbigvecs[6][2];
		extern msurface_t *r_skyfaces;
		switch (width) {
			case 1024:      VectorCopy (box_bigbigvecs[i][0], r_skytexinfo[i].vecs[0]); break;
			case 512:       VectorCopy (box_bigvecs[i][0], r_skytexinfo[i].vecs[0]); break;
			default:        VectorCopy (box_vecs[i][0], r_skytexinfo[i].vecs[0]); break;
		}
		switch (height) {
			case 1024:      VectorCopy (box_bigbigvecs[i][1], r_skytexinfo[i].vecs[1]); break;
			case 512:       VectorCopy (box_bigvecs[i][1], r_skytexinfo[i].vecs[1]); break;
			default:        VectorCopy (box_vecs[i][1], r_skytexinfo[i].vecs[1]); break;
		}
		// This is if one is already loaded and the size changed
		if (r_skyfaces) {
			r_skyfaces[i].texturemins[0] = -(width/2);
			r_skyfaces[i].texturemins[1] = -(height/2);
			r_skyfaces[i].extents[0] = width;
			r_skyfaces[i].extents[1] = height;
		}
		else Con_DPrintf ("Warning: No surface to load yet for WinQuake skybox");
		memcpy (r_skypixels[i], pic, width*height);
		pic = origpic;
		Hunk_FreeToLowMark (mark);
		if (final_pic != origpic)
			free(final_pic);
	}
	return true;
}

// Manoel Kasimier - skyboxes - begin
void R_InitSkyBox ()
{
	model_t *loadmodel = cl.worldmodel; // Manoel Kasimier - edited
	r_skyfaces = loadmodel->surfaces + loadmodel->numsurfaces;
	loadmodel->numsurfaces += 6;
	r_skyverts = loadmodel->vertexes + loadmodel->numvertexes;
	loadmodel->numvertexes += 8;
	r_skyedges = loadmodel->edges + loadmodel->numedges;
	loadmodel->numedges += 12;
	r_skysurfedges = loadmodel->surfedges + loadmodel->numsurfedges;
	loadmodel->numsurfedges += 24;
	memset (r_skyfaces, 0, 6 * sizeof(*r_skyfaces));
	for (int i = 0; i < 6; i++) {
		r_skyplanes[i].normal[skybox_planes[i*2]] = 1;
		r_skyplanes[i].dist = skybox_planes[i*2+1];
		r_skyfaces[i].plane = &r_skyplanes[i];
		r_skyfaces[i].numedges = 4;
		r_skyfaces[i].flags = box_faces[i] | SURF_DRAWSKYBOX;
		r_skyfaces[i].firstedge = loadmodel->numsurfedges-24+i*4;
		r_skyfaces[i].texinfo = &r_skytexinfo[i];
		// Manoel Kasimier - hi-res skyboxes - begin
		int width, height;
		if (r_skytexinfo[i].texture) {
			width = r_skytexinfo[i].texture->width;
			height = r_skytexinfo[i].texture->height;
		}
		else width = height = 256;
		switch (width)
		{
			case 1024:      VectorCopy (box_bigbigvecs[i][0], r_skytexinfo[i].vecs[0]); break;
			case 512:       VectorCopy (box_bigvecs[i][0], r_skytexinfo[i].vecs[0]); break;
			default:        VectorCopy (box_vecs[i][0], r_skytexinfo[i].vecs[0]); break;
		}

		switch (height) {
			case 1024:      VectorCopy (box_bigbigvecs[i][1], r_skytexinfo[i].vecs[1]); break;
			case 512:       VectorCopy (box_bigvecs[i][1], r_skytexinfo[i].vecs[1]); break;
			default:        VectorCopy (box_vecs[i][1], r_skytexinfo[i].vecs[1]); break;
		}
		r_skyfaces[i].texturemins[0] = -(width/2);
		r_skyfaces[i].texturemins[1] = -(height/2);
		r_skyfaces[i].extents[0] = width;
		r_skyfaces[i].extents[1] = height;
		// Manoel Kasimier - hi-res skyboxes - end
	}
	for(int i = 0; i < 24; i++)
		if (box_surfedges[i] > 0)
			r_skysurfedges[i] = loadmodel->numedges-13 + box_surfedges[i];
		else
			r_skysurfedges[i] = - (loadmodel->numedges-13 + -box_surfedges[i]);
	for(int i = 0; i < 12; i++) {
		r_skyedges[i].v[0] = loadmodel->numvertexes-9+box_edges[i*2+0];
		r_skyedges[i].v[1] = loadmodel->numvertexes-9+box_edges[i*2+1];
		r_skyedges[i].cachededgeoffset = 0;
	}
}

void R_EmitSkyBox ()
{
	if (insubmodel)
		return; // submodels should never have skies
	if (r_skyframe == r_framecount) {
		return; // already set this frame
	}
	r_skyframe = r_framecount;
	for (int i = 0 ; i < 8 ; i++) // set the eight fake vertexes
		for (int j = 0 ; j < 3 ; j++)
			r_skyverts[i].position[j] = r_origin[j] + box_verts[i][j]*128;
	for (int i = 0 ; i < 6 ; i++) // set the six fake planes
		if (skybox_planes[i*2+1] > 0)
			r_skyplanes[i].dist = r_origin[skybox_planes[i*2]]+128;
		else
			r_skyplanes[i].dist = r_origin[skybox_planes[i*2]]-128;
	for (int i = 0 ; i < 6; i++) { // fix texture offsets
		r_skytexinfo[i].vecs[0][3] = -DotProduct (r_origin, r_skytexinfo[i].vecs[0]);
		r_skytexinfo[i].vecs[1][3] = -DotProduct (r_origin, r_skytexinfo[i].vecs[1]);
	}
	int oldkey = r_currentkey; // emit the six faces
	r_currentkey = 0x7ffffff0;
	for (int i = 0; i < 6; i++)
		R_RenderFace (r_skyfaces + i, 15);
	r_currentkey = oldkey; // bsp sorting order
} // Manoel Kasimier - skyboxes - end

void Sky_NewMap()
{ // read worldspawn (this is so ugly, and shouldn't it be done on the server?)
	char key[128], value[4096];
	const char *data = cl.worldmodel->entities;
	data = COM_Parse(data);
	if (!data || com_token[0] != '{') // should never happen
		return; // error
	while (1) {
		data = COM_Parse(data);
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
		if (!strcmp("sky", key))
			Sky_LoadSkyBox(value);
		if (!strcmp("skyfog", key))
		        Cvar_SetValue("r_skyfog", atof(value)); // also accept non-standard keys
		else if (!strcmp("skyname", key)) // half-life
			Sky_LoadSkyBox(value);
		else if (!strcmp("qlsky", key)) // quake lives
			Sky_LoadSkyBox(value);
	}
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
	Cmd_AddCommand ("sky", Sky_SkyCommand_f);
	Cvar_RegisterVariable (&r_skyfog);
        skybox_name[0] = 0;
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

void R_SetSkyFrame ()
{
        skyspeed = iskyspeed;
        skyspeed2 = iskyspeed2;
        int g = GreatestCommonDivisor (iskyspeed, iskyspeed2);
        int s1 = iskyspeed / g;
        int s2 = iskyspeed2 / g;
        float temp = SKYSIZE * s1 * s2;
        skytime = cl.time - ((int)(cl.time / temp) * temp);
}
