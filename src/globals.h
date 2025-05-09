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
EX u32	lfsr; // non-zero seed
EX f32	fog_density;
EX f32	fog_red;   // CyanBun96: we store the actual RGB values in these,
EX f32	fog_green; // but they get quantized to a single index in the color
EX f32	fog_blue;  // palette before use, stored in fog_pal_index
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
EX s8 modelist[NUM_OLDMODES][8]; // "320x240" etc. for menus
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
EX u8 vid_curpal[256 * 3]; // save for mode changes
EX viddef_t vid; // global video state
EX cvar_t _vid_default_mode_win;
EX cvar_t _windowed_mouse;
EX cvar_t scr_uiscale;
EX cvar_t newoptions;
EX cvar_t sensitivityyscale;
EX s8 *VID_GetModeDescription(s32 mode);
EX void VID_SetPalette(u8 *palette, SDL_Surface *dest);
EX void VID_Init(u8 *palette);
EX void VID_Shutdown();
EX void VID_CalcScreenDimensions(cvar_t *cvar);
EX void VID_RenderFrame();
EX void VID_SetMode(s32 moden, s32 custw, s32 custh, s32 custwinm, u8 *pal);
EX void VID_Update();

#undef EX
