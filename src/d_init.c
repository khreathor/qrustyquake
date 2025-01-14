// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

// d_init.c: rasterization driver initialization

#include "quakedef.h"
#include "d_local.h"

#define NUM_MIPS	4

cvar_t d_subdiv16 = { "d_subdiv16", "1", false, false, 0, NULL };
cvar_t d_mipcap = { "d_mipcap", "0", false, false, 0, NULL };
cvar_t d_mipscale = { "d_mipscale", "1", false, false, 0, NULL };
surfcache_t *d_initial_rover;
qboolean d_roverwrapped;
int d_minmip;
float d_scalemip[NUM_MIPS - 1];
static float basemip[NUM_MIPS - 1] = { 1.0, 0.5 * 0.8, 0.25 * 0.8 };
extern int d_aflatcolor;
void (*d_drawspans)(espan_t * pspan);

void D_Init()
{
	Cvar_RegisterVariable(&d_subdiv16);
	Cvar_RegisterVariable(&d_mipcap);
	Cvar_RegisterVariable(&d_mipscale);
	r_drawpolys = false;
	r_worldpolysbacktofront = false;
	r_recursiveaffinetriangles = true;
	r_pixbytes = 1;
	r_aliasuvscale = 1.0;
}

void D_SetupFrame()
{
	d_viewbuffer = r_dowarp ? r_warpbuffer : (void *)vid.buffer;
	screenwidth = r_dowarp ? WARP_WIDTH : vid.rowbytes;
	d_roverwrapped = false;
	d_initial_rover = sc_rover;
	d_minmip = d_mipcap.value;
	d_minmip = CLAMP(0, d_minmip, 3);
	for (int i = 0; i < (NUM_MIPS - 1); i++)
		d_scalemip[i] = basemip[i] * d_mipscale.value;
	d_drawspans = D_DrawSpans8;
	d_aflatcolor = 0;
}
