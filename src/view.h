// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

extern cvar_t v_gamma;
extern byte gammatable[256];	// palette is sent through this
extern byte ramps[3][256];
extern float v_blend[4];
extern cvar_t lcd_x;

void V_Init ();
void V_RenderView ();
float V_CalcRoll (vec3_t angles, vec3_t velocity);
void V_UpdatePalette ();
