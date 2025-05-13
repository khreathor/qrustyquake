#include "quakedef.h"

u8 color_mix_lut[256][256][FOG_LUT_LEVELS];                          // rgbtoi.c
s32 fog_lut_built = 0;
f32 gamma_lut[256];
s32 color_conv_initialized = 0;
lab_t lab_palette[256];
u8 lit_lut[LIT_LUT_RES*LIT_LUT_RES*LIT_LUT_RES];
u8 lit_lut_initialized = 0;
u8 rgb_lut[RGB_LUT_SIZE];
s32 rgb_lut_built = 0;

s32 fog_initialized = 0;                                              // d_fog.c
u32 lfsr = 0x1337; // non-zero seed
f32 fog_density;
f32 fog_red; // CyanBun96: we store the actual RGB values in these,
f32 fog_green; // but they get quantized to a single index in the color
f32 fog_blue; // palette before use, stored in fog_pal_index
u8 fog_pal_index;
f32 randarr[RANDARR_SIZE];

u32 oldmodes[NUM_OLDMODES * 2] = {                                  // vid_sdl.c
        320, 240,       640, 480,       800, 600,
        320, 200,       320, 240,       640, 350,
        640, 400,       640, 480,       800, 600 };
SDL_Window *window;
SDL_Surface *windowSurface;
SDL_Renderer *renderer;
SDL_Surface *argbbuffer;
SDL_Texture *texture;
SDL_Rect blitRect;
SDL_Rect destRect;
SDL_Surface *scaleBuffer;
SDL_Surface *screen; // the main video buffer
SDL_Surface *screen1; // used for scr_centerstring only ATM
s8 modelist[NUM_OLDMODES][8]; // "320x240" etc. for menus
u32 force_old_render = 0;
u32 SDLWindowFlags;
u32 uiscale;
u32 vimmode;
s32 vid_line;
s32 vid_modenum;
s32 vid_testingmode;
s32 vid_realmode;
s32 vid_default;
f64 vid_testendtime;
u8 vid_curpal[256 * 3];
s32 VID_highhunkmark;
viddef_t vid; // global video state

u8 r_foundtranswater, r_wateralphapass;                            // r_main.c
s32 r_pass; // CyanBun96: 1 - cutout textures 0 - everything else
void *colormap;
vec3_t viewlightvec;
alight_t r_viewlighting = { 128, 192, viewlightvec };
f32 r_time1;
s32 r_numallocatededges;
bool r_recursiveaffinetriangles = true;
s32 r_pixbytes = 1;
f32 r_aliasuvscale = 1.0;
s32 r_outofsurfaces;
s32 r_outofedges;
bool r_dowarp, r_dowarpold, r_viewchanged;
s32 numbtofpolys;
btofpoly_t *pbtofpolys;
mvertex_t *r_pcurrentvertbase;
s32 c_surf;
s32 r_maxsurfsseen, r_maxedgesseen, r_cnumsurfs;
bool r_surfsonstack;
s32 r_clipflags;
u8 *r_warpbuffer;
bool r_fov_greater_than_90;
vec3_t vup, base_vup; // view origin
vec3_t vpn, base_vpn;
vec3_t vright, base_vright;
vec3_t r_origin;
refdef_t r_refdef; // screen size info
f32 xcenter, ycenter;
f32 xscale, yscale;
f32 xscaleinv, yscaleinv;
f32 xscaleshrink, yscaleshrink;
f32 aliasxscale, aliasyscale, aliasxcenter, aliasycenter;
s32 screenwidth;
f32 pixelAspect;
f32 screenAspect;
f32 verticalFieldOfView;
f32 xOrigin, yOrigin;
mplane_t screenedge[4];
u32 r_visframecount; // refresh flags
u32 r_framecount = 1; // so frame counts initialized to 0 don't match
s32 d_spanpixcount;
s32 r_polycount;
s32 r_drawnpolycount;
s32 r_wholepolycount;
s8 viewmodname[VIEWMODNAME_LENGTH + 1];
s32 modcount;
s32 *pfrustum_indexes[4];
s32 r_frustum_indexes[4 * 6];
s32 reinit_surfcache = 1; // if 1, surface cache is currently empty and
			// must be reinitialized for current cache size
mleaf_t *r_viewleaf, *r_oldviewleaf;
texture_t *r_notexture_mip;
f32 r_aliastransition, r_resfudge;
s32 d_lightstylevalue[256]; // 8.8 fraction of base light value
f32 dp_time1, dp_time2, db_time1, db_time2, rw_time1, rw_time2;
f32 se_time1, se_time2, de_time1, de_time2, dv_time1, dv_time2;
s32 colored_aliaslight;

f32 d_sdivzstepu, d_tdivzstepu, d_zistepu;                         // d_vars.c
f32 d_sdivzstepv, d_tdivzstepv, d_zistepv;
f32 d_sdivzorigin, d_tdivzorigin, d_ziorigin;
s32 sadjust, tadjust, bbextents, bbextentt;
u8 *cacheblock;
s32 cachewidth;
u8 *d_viewbuffer;
s16 *d_pzbuffer;
u32 d_zrowbytes;
u32 d_zwidth;
