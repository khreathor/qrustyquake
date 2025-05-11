#include "quakedef.h"

typedef unsigned char byte; // TODO remove
typedef unsigned char pixel_t;
typedef float f32;
#ifndef QGLOBALS_
#define QGLOBALS_

#define EX extern // globals.h only
#define FOG_LUT_LEVELS 32                                            // rgbtoi.c
#define LIT_LUT_RES 64
EX u8	color_mix_lut[256][256][FOG_LUT_LEVELS];
EX u8	lit_lut[LIT_LUT_RES*LIT_LUT_RES*LIT_LUT_RES];
EX u8	lit_lut_initialized;
EX s32	fog_lut_built;
EX f32	gamma_lut[256];
EX s32	color_conv_initialized;
EX lab_t lab_palette[256];
EX void init_color_conv();
EX u8 rgbtoi_lab(u8 r, u8 g, u8 b);
EX u8 rgbtoi(u8 r, u8 g, u8 b);
EX void build_color_mix_lut(cvar_t *cvar);
EX void R_BuildLitLUT();

#define RANDARR_SIZE 19937 // prime to reduce unintended patterns     // d_fog.c
EX s32	fog_initialized;
EX u32	lfsr;
EX f32	fog_density;
EX f32	fog_red;
EX f32	fog_green;
EX f32	fog_blue;
EX u8	fog_pal_index;
EX f32	randarr[RANDARR_SIZE];

#define NUM_OLDMODES 9                                              // vid_sdl.c
#define VID_GRADES (1 << VID_CBITS)
#define VID_CBITS 6
#define MAX_MODE_LIST 30
#define VID_ROW_SIZE 3
#define MAX_COLUMN_SIZE 5
#define MODE_AREA_HEIGHT (MAX_COLUMN_SIZE + 6)
EX u32 oldmodes[NUM_OLDMODES*2];
EX s8 modelist[NUM_OLDMODES][8];
EX SDL_Window *window;
EX SDL_Surface *windowSurface;
EX SDL_Renderer *renderer;
EX SDL_Surface *argbbuffer;
EX SDL_Texture *texture;
EX SDL_Rect blitRect;
EX SDL_Rect destRect;
EX SDL_Surface *scaleBuffer;
EX SDL_Surface *screen;
EX SDL_Surface *screen1;
EX u32 force_old_render;
EX u32 SDLWindowFlags;
EX u32 uiscale;
EX u32 vimmode;
EX s32 vid_line;
EX s32 vid_modenum;
EX s32 vid_testingmode;
EX s32 vid_realmode;
EX s32 vid_default;
EX f64 vid_testendtime;
EX u8 vid_curpal[256 * 3];
EX viddef_t vid;
EX cvar_t _vid_default_mode_win;
EX cvar_t _windowed_mouse;
EX cvar_t scr_uiscale;
EX cvar_t newoptions;
EX cvar_t sensitivityyscale;
EX s32 VID_highhunkmark;
EX s8 *VID_GetModeDescription(s32 mode);
EX void VID_SetPalette(u8 *palette, SDL_Surface *dest);
EX void VID_Init(u8 *palette);
EX void VID_Shutdown();
EX void VID_CalcScreenDimensions(cvar_t *cvar);
EX void VID_RenderFrame();
EX void VID_SetMode(s32 moden, s32 custw, s32 custh, s32 custwinm, u8 *pal);
EX void VID_Update();

#define	VIEWMODNAME_LENGTH 256                                       // r_main.c
EX byte r_foundtranswater, r_wateralphapass;
EX s32 r_pass;
EX void *colormap;
EX vec3_t viewlightvec;
EX alight_t r_viewlighting;
EX f32 r_time1;
EX s32 r_numallocatededges;
EX bool r_recursiveaffinetriangles;
EX s32 r_pixbytes;
EX f32 r_aliasuvscale;
EX s32 r_outofsurfaces;
EX s32 r_outofedges;
EX bool r_dowarp, r_dowarpold, r_viewchanged;
EX s32 numbtofpolys;
EX btofpoly_t *pbtofpolys;
EX mvertex_t *r_pcurrentvertbase;
EX s32 c_surf;
EX s32 r_maxsurfsseen, r_maxedgesseen, r_cnumsurfs;
EX bool r_surfsonstack;
EX s32 r_clipflags;
EX byte *r_warpbuffer;
EX bool r_fov_greater_than_90;
EX vec3_t vup, base_vup;
EX vec3_t vpn, base_vpn;
EX vec3_t vright, base_vright;
EX vec3_t r_origin;
EX refdef_t r_refdef;
EX f32 xcenter, ycenter;
EX f32 xscale, yscale;
EX f32 xscaleinv, yscaleinv;
EX f32 xscaleshrink, yscaleshrink;
EX f32 aliasxscale, aliasyscale, aliasxcenter, aliasycenter;
EX s32 screenwidth;
EX f32 pixelAspect;
EX f32 screenAspect;
EX f32 verticalFieldOfView;
EX f32 xOrigin, yOrigin;
EX mplane_t screenedge[4];
EX u32 r_visframecount;
EX u32 r_framecount;
EX s32 d_spanpixcount;
EX s32 r_polycount;
EX s32 r_drawnpolycount;
EX s32 r_wholepolycount;
EX s8 viewmodname[VIEWMODNAME_LENGTH + 1];
EX s32 modcount;
EX s32 *pfrustum_indexes[4];
EX s32 r_frustum_indexes[4 * 6];
EX s32 reinit_surfcache;
EX mleaf_t *r_viewleaf, *r_oldviewleaf;
EX texture_t *r_notexture_mip;
EX f32 r_aliastransition, r_resfudge;
EX s32 d_lightstylevalue[256];
EX f32 dp_time1, dp_time2, db_time1, db_time2, rw_time1, rw_time2;
EX f32 se_time1, se_time2, de_time1, de_time2, dv_time1, dv_time2;
EX s32 colored_aliaslight;
#undef EX
#endif
