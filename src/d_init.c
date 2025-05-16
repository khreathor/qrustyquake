// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.
// d_init.c: rasterization driver initialization
#include "quakedef.h"

static f32 basemip[NUM_MIPS - 1] = { 1.0, 0.5 * 0.8, 0.25 * 0.8 };

void D_Init()
{
	Cvar_RegisterVariable(&d_mipcap);
	Cvar_RegisterVariable(&d_mipscale);
}

void D_SetupFrame()
{
	d_viewbuffer = r_dowarp ? r_warpbuffer : (void *)vid.buffer;
	screenwidth = r_dowarp ? WARP_WIDTH : vid.width;
	d_roverwrapped = false;
	d_initial_rover = sc_rover;
	d_minmip = d_mipcap.value;
	d_minmip = CLAMP(0, d_minmip, 3);
	for (s32 i = 0; i < (NUM_MIPS - 1); i++)
		d_scalemip[i] = basemip[i] * d_mipscale.value;
}
