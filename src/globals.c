#include "quakedef.h"

u8 color_mix_lut[256][256][FOG_LUT_LEVELS];                          // rgbtoi.c
s32 fog_lut_built = 0;
f32 gamma_lut[256];
s32 color_conv_initialized = 0;
lab_t lab_palette[256];
u8 lit_lut[LIT_LUT_RES*LIT_LUT_RES*LIT_LUT_RES];
u8 lit_lut_initialized = 0;

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

