#include "quakedef.h"

#ifndef QGLOBALS_
#define QGLOBALS_

#define EX extern // globals.h only

EX u8 color_mix_lut[256][256][FOG_LUT_LEVELS];                       // rgbtoi.c
EX u8 lit_lut[LIT_LUT_RES*LIT_LUT_RES*LIT_LUT_RES];
EX u8 lit_lut_initialized;
EX s32 fog_lut_built;
EX f32 gamma_lut[256];
EX s32 color_conv_initialized;
EX lab_t lab_palette[256];
EX void init_color_conv();
EX u8 rgbtoi_lab(u8 r, u8 g, u8 b);
EX u8 rgbtoi(u8 r, u8 g, u8 b);
EX void build_color_mix_lut(cvar_t *cvar);
EX void R_BuildLitLUT();

EX s32 fog_initialized;                                               // d_fog.c
EX u32 lfsr;
EX f32 fog_density;
EX f32 fog_red;
EX f32 fog_green;
EX f32 fog_blue;
EX u8 fog_pal_index;
EX f32 randarr[RANDARR_SIZE];

EX u32 oldmodes[NUM_OLDMODES*2];                                    // vid_sdl.c
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
EX void VID_VidSetModeCommand_f();

EX byte r_foundtranswater, r_wateralphapass;                         // r_main.c
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

EX s32 m_activenet;                                                    // menu.h
EX void M_Init ();
EX void M_Keydown (s32 key);
EX void M_Draw ();
EX void M_ToggleMenu_f ();

EX s32 vgatext_main(SDL_Window *window, Uint16 *screen);            // vgatext.h

EX byte gammatable[256]; // palette is sent through this               // view.h
EX byte ramps[3][256];
EX f32 v_blend[4];
EX void V_Init ();
EX void V_RenderView ();
EX f32 V_CalcRoll (vec3_t angles, vec3_t velocity);
EX void V_UpdatePalette ();

EX void IN_Init ();                                                   // input.h
EX void IN_Shutdown ();
EX void IN_Commands ();
EX void IN_Move (usercmd_t *cmd);
EX void IN_ClearStates ();
EX void Sys_SendKeyEvents ();

EX void CRC_Init(u16 *crcvalue);                                        // crc.h
EX void CRC_ProcessByte(u16 *crcvalue, byte data);
EX u16 CRC_Value(u16 crcvalue);
EX u16 CRC_Block (const byte *start, s32 count); //johnfitz -- texture crc

EX keydest_t key_dest;                                                 // keys.h
EX s8 *keybindings[256];
EX s32 key_repeats[256];
EX s32 key_count; // incremented every key event
EX s32 key_lastpress;
EX void Key_Event(s32 key, bool down);
EX void Key_Init();
EX void Key_WriteBindings(FILE *f);
EX void Key_SetBinding(s32 keynum, s8 *binding);

EX s32 sb_lines; // scan lines to draw                                 // sbar.h
EX void Sbar_Init();
EX void Sbar_Changed();
EX void Sbar_Draw(); // called every frame by screen
EX void Sbar_IntermissionOverlay();
EX void Sbar_FinaleOverlay();
EX void Sbar_CalcPos(); // call on each sbar size change

EX s32 Sys_FileType (const s8 *path); // File IO                        // sys.h
EX s32 Sys_FileOpenRead (const s8 *path, s32 *hndl);
EX s32 Sys_FileOpenWrite (const s8 *path);
EX void Sys_FileClose (s32 handle);
EX void Sys_FileSeek (s32 handle, s32 position);
EX s32 Sys_FileRead (s32 handle, void *dest, s32 count);
EX s32 Sys_FileWrite (s32 handle, const void *data, s32 count);
EX s32 Sys_FileTime (const s8 *path);
EX void Sys_mkdir (const s8 *path);
EX void Sys_Error (const s8 *error, ...); // System IO
EX void Sys_Printf (const s8 *fmt, ...);
EX void Sys_Quit ();
EX f64 Sys_FloatTime ();
EX f64 Sys_DoubleTime ();

s32 Loop_Init ();                                                  // net_loop.h
void Loop_Listen (bool state);
void Loop_SearchForHosts (bool xmit);
qsocket_t *Loop_Connect (const char *host);
qsocket_t *Loop_CheckNewConnections ();
s32 Loop_GetMessage (qsocket_t *sock);
s32 Loop_SendMessage (qsocket_t *sock, sizebuf_t *data);
s32 Loop_SendUnreliableMessage (qsocket_t *sock, sizebuf_t *data);
bool Loop_CanSendMessage (qsocket_t *sock);
bool Loop_CanSendUnreliableMessage (qsocket_t *sock);
void Loop_Close (qsocket_t *sock);
void Loop_Shutdown ();

EX qsocket_t *net_activeSockets;                                   // net_defs.h
EX qsocket_t *net_freeSockets;
EX int net_numsockets;
EX net_landriver_t net_landrivers[];
EX const int net_numlandrivers;
EX net_driver_t net_drivers[];
EX const int net_numdrivers;
EX int net_driverlevel;
EX int messagesSent;
EX int messagesReceived;
EX int unreliableMessagesSent;
EX int unreliableMessagesReceived;
EX int hostCacheCount;
EX hostcache_t hostcache[HOSTCACHESIZE];
qsocket_t *NET_NewQSocket ();
void NET_FreeQSocket(qsocket_t *);
double SetNetTime();
void SchedulePollProcedure(PollProcedure *pp, double timeOffset);
#undef EX
#endif
