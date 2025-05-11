// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

// d_iface.h: interface header file for rasterization driver modules

extern int d_spanpixcount;
extern float r_aliasuvscale; // scale-up factor for screen u and on Alias vertices passed to driver
extern int r_pixbytes;
extern affinetridesc_t r_affinetridesc;
extern spritedesc_t r_spritedesc;
extern zpointdesc_t r_zpointdesc;
extern polydesc_t r_polydesc;
extern int d_con_indirect; // if 0, Quake will draw console directly to vid.buffer; if 1, Quake will draw console via D_DrawRect. Must be defined by driver
extern vec3_t r_pright, r_pup, r_ppn;
extern void *acolormap; // FIXME: should go away
extern drawsurf_t r_drawsurf;
extern float skyspeed, skyspeed2;
extern float skytime;
extern int c_surf;
extern vrect_t scr_vrect;
extern byte *r_warpbuffer;

// this is currently for internal use only, and should not be used by drivers
extern byte *r_skysource;

void D_BeginDirectRect(int x, int y, byte *pbitmap, int width, int height);
void D_PolysetDraw();
void D_PolysetDrawFinalVerts(finalvert_t *fv, int numverts);
void D_DrawParticle(particle_t *pparticle);
void D_DrawPoly();
void D_DrawSprite();
void D_DrawSurfaces();
void D_Init();
void D_ViewChanged();
void D_SetupFrame();
void D_TurnZOn();
void D_WarpScreen();
void D_FillRect(vrect_t *vrect, int color);
void D_DrawRect();
void R_DrawSurface();
// currently for internal use only, and should be a do-nothing function in
// hardware drivers
// FIXME: this should go away
void D_PolysetUpdateTables();
