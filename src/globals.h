#include "quakedef.h"

#ifndef QGLOBALS_
#define QGLOBALS_

#define EX extern // globals.h only
extern double host_time;                                             // quakedef
extern bool noclip_anglehack;
extern quakeparms_t host_parms;
extern bool host_initialized; // true if into command execution
extern double host_frametime;
extern byte *host_basepal;
extern byte *host_colormap;
extern bool isDedicated;
extern int minimum_memory;
extern int host_framecount; // incremented every frame, never reset
extern double realtime; //  never reset
extern bool msg_suppress_1;
extern int current_skill;
void Host_ClearMemory();
void Host_ServerFrame();
void Host_InitCommands();
void Host_Init();
void Host_Shutdown();
void Host_Error(char *error, ...);
void Host_EndGame(char *message, ...);
void Host_Frame(float time);
void Host_Quit_f();
void Host_ClientCommands(char *fmt, ...);
void Host_ShutdownServer(bool crash);
void Chase_Init();
void Chase_Update();
void Cvar_SetCallback(cvar_t *var, cvarcallback_t func);

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

extern int con_totallines;                                          // console.h
extern int con_backscroll;
extern bool con_forcedup; // because no entities to refresh
extern bool con_initialized;
extern byte *con_chars;
extern int con_notifylines; // scan lines to clear for notify lines
void Con_DrawCharacter (int cx, int line, int num);
void Con_CheckResize (void);
void Con_Init (void);
void Con_DrawConsole (int lines, bool drawinput);
void Con_Print (char *txt);
void Con_Printf (const char *fmt, ...);
void Con_DPrintf (char *fmt, ...);
void Con_SafePrintf (char *fmt, ...);
void Con_Clear_f (void);
void Con_DrawNotify (void);
void Con_ClearNotify (void);
void Con_ToggleConsole_f (void);

int Datagram_Init ();                                              // net_dgrm.h
void Datagram_Listen (bool state);
void Datagram_SearchForHosts (bool xmit);
qsocket_t *Datagram_Connect (const char *host);
qsocket_t *Datagram_CheckNewConnections ();
int Datagram_GetMessage (qsocket_t *sock);
int Datagram_SendMessage (qsocket_t *sock, sizebuf_t *data);
int Datagram_SendUnreliableMessage (qsocket_t *sock, sizebuf_t *data);
bool Datagram_CanSendMessage (qsocket_t *sock);
bool Datagram_CanSendUnreliableMessage (qsocket_t *sock);
void Datagram_Close (qsocket_t *sock);
void Datagram_Shutdown ();

extern qpic_t *draw_disc; // also used on sbar                         // draw.h
void Draw_Init ();
void Draw_CharacterScaled (int x, int y, int num, int scale);
void Draw_PicScaled (int x, int y, qpic_t *pic, int scale);
void Draw_PicScaledPartial(int x,int y,int l,int t,int w,int h,qpic_t *p,int s);
void Draw_TransPicScaled (int x, int y, qpic_t *pic, int scale);
void Draw_TransPicTranslateScaled(int x, int y, qpic_t *p, byte *tl, int scale);
void Draw_ConsoleBackground (int lines);
void Draw_TileClear (int x, int y, int w, int h);
void Draw_Fill (int x, int y, int w, int h, int c);
void Draw_FadeScreen ();
void Draw_StringScaled (int x, int y, char *str, int scale);
qpic_t *Draw_PicFromWad (char *name);
qpic_t *Draw_CachePic (char *path);

extern aliashdr_t *pheader;                                           // model.h
extern stvert_t stverts[MAXALIASVERTS];
extern mtriangle_t triangles[MAXALIASTRIS];
extern trivertx_t *poseverts[MAXALIASFRAMES];
void Mod_Init ();
void Mod_ClearAll ();
void Mod_ResetAll (); // for gamedir changes (Host_Game_f)
model_t *Mod_ForName (const s8 *name, bool crash);
void *Mod_Extradata (model_t *mod); // handles caching
void Mod_TouchModel (const s8 *name);
mleaf_t *Mod_PointInLeaf (vec3_t p, model_t *model);
byte *Mod_LeafPVS (mleaf_t *leaf, model_t *model);
byte *Mod_NoVisPVS (model_t *model);
void Mod_SetExtraFlags (model_t *mod);

void SCR_Init ();                                                    // screen.h
void SCR_UpdateScreen ();
void SCR_SizeUp ();
void SCR_SizeDown ();
void SCR_CenterPrint (const char *str);
void SCR_BeginLoadingPlaque ();
void SCR_EndLoadingPlaque ();
int SCR_ModalMessage (char *text);
extern float scr_con_current;
extern float scr_conlines; // lines of console to display
extern int sb_lines;
extern unsigned int scr_fullupdate; // set to 0 to force full redraw
extern unsigned int clearnotify; // set to 0 whenever notify text is drawn
extern bool scr_disabled_for_loading;
extern bool scr_skipupdate;
extern bool block_drawing;
extern hudstyle_t hudstyle;

sys_socket_t UDP_Init();                                            // net_udp.h
void UDP_Shutdown();
void UDP_Listen(bool state);
sys_socket_t UDP_OpenSocket(int port);
int UDP_CloseSocket(sys_socket_t socketid);
int UDP_Connect(sys_socket_t socketid, struct qsockaddr *addr);
sys_socket_t UDP_CheckNewConnections();
int UDP_Read(sys_socket_t socketid, byte *buf, int len, struct qsockaddr *addr);
int UDP_Write(sys_socket_t socketid, byte *buf, int len, struct qsockaddr *addr);
int UDP_Broadcast(sys_socket_t socketid, byte *buf, int len);
const char *UDP_AddrToString(struct qsockaddr *addr);
int UDP_StringToAddr(const char *string, struct qsockaddr *addr);
int UDP_GetSocketAddr(sys_socket_t socketid, struct qsockaddr *addr);
int UDP_GetNameFromAddr(struct qsockaddr *addr, char *name);
int UDP_GetAddrFromName(const char *name, struct qsockaddr *addr);
int UDP_AddrCompare(struct qsockaddr *addr1, struct qsockaddr *addr2);
int UDP_GetSocketPort(struct qsockaddr *addr);
int UDP_SetSocketPort(struct qsockaddr *addr, int port);

extern int wad_numlumps;                                                // wad.h
extern lumpinfo_t *wad_lumps;
extern byte *wad_base;
void W_LoadWadFile (void); //johnfitz -- filename is now hard-coded
void W_CleanupName (const char *in, char *out);
void *W_GetLumpName (const char *name);
void *W_GetLumpNum (int num);
wad_t *W_LoadWadList (const char *names);
void W_FreeWadList (wad_t *wads);
lumpinfo_t *W_GetLumpinfoList (wad_t *wads, const char *name, wad_t **out_wad);
void SwapPic (qpic_t *pic);

extern int reinit_surfcache;                                         // render.h
extern vec3_t r_origin, vpn, vright, vup;
extern int reinit_surfcache;
extern bool r_cache_thrash;
void R_Init();
void R_InitTextures();
void R_InitEfrags();
void R_RenderView();
void R_ViewChanged(vrect_t *pvrect, int lineadj, float aspect);
void R_InitSky(struct texture_s *mt);
void R_AddEfrags(entity_t *ent);
void R_RemoveEfrags(entity_t *ent);
void R_NewMap();
void R_ParseParticleEffect();
void R_RunParticleEffect(vec3_t org, vec3_t dir, int color, int count);
void R_RocketTrail(vec3_t start, vec3_t end, int type);
void R_EntityParticles(entity_t *ent);
void R_BlobExplosion(vec3_t org);
void R_ParticleExplosion(vec3_t org);
void R_ParticleExplosion2(vec3_t org, int colorStart, int colorLength);
void R_LavaSplash(vec3_t org);
void R_TeleportSplash(vec3_t org);
void R_PushDlights();
int D_SurfaceCacheForRes(int width, int height);
void D_FlushCaches();
void D_DeleteSurfaceCache();
void D_InitCaches(void *buffer, int size);
void R_SetVrect(vrect_t *pvrect, vrect_t *pvrectin, int lineadj);

extern int d_spanpixcount;                                          // d_iface.h
extern float r_aliasuvscale;
extern int r_pixbytes;
extern affinetridesc_t r_affinetridesc;
extern spritedesc_t r_spritedesc;
extern zpointdesc_t r_zpointdesc;
extern polydesc_t r_polydesc;
extern int d_con_indirect;
extern vec3_t r_pright, r_pup, r_ppn;
extern void *acolormap;
extern drawsurf_t r_drawsurf;
extern float skyspeed, skyspeed2;
extern float skytime;
extern int c_surf;
extern vrect_t scr_vrect;
extern byte *r_warpbuffer;
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
void D_PolysetUpdateTables();

extern vec3_t vec3_origin;                                          // mathlib.h
extern int nanmask;
void VectorMA(vec3_t veca, float scale, vec3_t vecb, vec3_t vecc);
vec_t Length(vec3_t v);
void CrossProduct(vec3_t v1, vec3_t v2, vec3_t cross);
float VectorNormalize(vec3_t v); // returns vector length
void VectorInverse(vec3_t v);
void VectorScale(vec3_t in, vec_t scale, vec3_t out);
void R_ConcatRotations(float in1[3][3], float in2[3][3], float out[3][3]);
void R_ConcatTransforms(float in1[3][4], float in2[3][4], float out[3][4]);
void FloorDivMod(double numer, double denom, int *quotient, int *rem);
int GreatestCommonDivisor(int i1, int i2);
void AngleVectors(vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
int BoxOnPlaneSide(vec3_t emins, vec3_t emaxs, struct mplane_s *plane);
float anglemod(float a);
vec_t VectorLength(vec3_t v);
void VectorSubtract(const vec3_t a, const vec3_t b, vec3_t c);
void VectorAdd(const vec3_t a, const vec3_t b, vec3_t c);
void VectorCopy(const vec3_t a, vec3_t b);

extern int cachewidth;                                             // r_shared.h
extern pixel_t *cacheblock;
extern int screenwidth;
extern float pixelAspect;
extern int r_drawnpolycount;
extern int sintable[SIN_BUFFER_SIZE];
extern int intsintable[SIN_BUFFER_SIZE];
extern vec3_t vup, base_vup;
extern vec3_t vpn, base_vpn;
extern vec3_t vright, base_vright;
extern entity_t *currententity;
extern vec3_t sxformaxis[4]; // s axis transformed into viewspace
extern vec3_t txformaxis[4]; // t axis transformed into viewspac
extern vec3_t modelorg, base_modelorg;
extern float xcenter, ycenter;
extern float xscale, yscale;
extern float xscaleinv, yscaleinv;
extern float xscaleshrink, yscaleshrink;
extern int d_lightstylevalue[256]; // 8.8 frac of base light value
extern int r_skymade;
extern char skybox_name[1024];
extern int ubasestep, errorterm, erroradjustup, erroradjustdown;
extern surf_t *surfaces, *surface_p, *surf_max;
extern void TransformVector(vec3_t in, vec3_t out);
extern void SetUpForLineScan(fixed8_t startvertu, fixed8_t startvertv,
		fixed8_t endvertu, fixed8_t endvertv);
extern void R_MakeSky();
extern void R_DrawLine(polyvert_t *polyvert0, polyvert_t *polyvert1);

void SV_ClearWorld();                                                 // world.h
void SV_UnlinkEdict(edict_t *ent);
void SV_LinkEdict(edict_t *ent, bool touch_triggers);
int SV_PointContents(vec3_t p);
int SV_TruePointContents(vec3_t p);
edict_t *SV_TestEntityPosition(edict_t *ent);
trace_t SV_Move(vec3_t start, vec3_t mins, vec3_t maxs,
		vec3_t end, int type, edict_t *passedict);

extern float scale_for_mip;                                         // d_local.h
extern bool d_roverwrapped;
extern surfcache_t *sc_rover;
extern surfcache_t *d_initial_rover;
extern float d_sdivzstepu, d_tdivzstepu, d_zistepu;
extern float d_sdivzstepv, d_tdivzstepv, d_zistepv;
extern float d_sdivzorigin, d_tdivzorigin, d_ziorigin;
extern fixed16_t sadjust, tadjust;
extern fixed16_t bbextents, bbextentt;
extern short *d_pzbuffer;
extern unsigned int d_zrowbytes, d_zwidth;
extern int *d_pscantable;
extern int d_scantable[MAXHEIGHT];
extern int d_vrectx, d_vrecty, d_vrectright_particle, d_vrectbottom_particle;
extern int d_y_aspect_shift, d_pix_min, d_pix_max, d_pix_shift;
extern pixel_t *d_viewbuffer;
extern short *zspantable[MAXHEIGHT];
extern int d_minmip;
extern float d_scalemip[3];
extern int r_pass;
extern float winquake_surface_liquid_alpha;
extern unsigned char color_mix_lut[256][256][FOG_LUT_LEVELS];
extern unsigned char fog_pal_index;
extern int lmonly;
extern unsigned char *litwater_base;
extern int lwmark;
void D_DrawSpans8(espan_t *pspans);
void D_DrawTransSpans8(espan_t *pspans, float opacity);
void D_DrawZSpans(espan_t *pspans);
void Turbulent8(espan_t *pspan, float opacity);
void D_SpriteDrawSpans(sspan_t *pspan);
void D_DrawSkyScans8(espan_t *pspan);
void D_DrawSkyboxScans8(espan_t *pspans);
void R_ShowSubDiv();
extern void(*prealspandrawer)();
surfcache_t *D_CacheSurface(msurface_t *surface, int miplevel);
extern int D_MipLevelForScale(float scale);
extern int dither_pat;
extern int D_Dither(byte *pos);

extern int DEFAULTnet_hostport;                                         // net.h
extern int net_hostport;
extern double net_time;
extern sizebuf_t net_message;
extern int net_activeconnections;
extern bool slistInProgress;
extern bool slistSilent;
extern bool slistLocal;
extern int hostCacheCount;
extern int ipxAvailable;
extern bool tcpipAvailable;
extern char my_ipx_address[NET_NAMELEN];
extern char my_tcpip_address[NET_NAMELEN];
void NET_Init();
void NET_Shutdown();
struct qsocket_s *NET_CheckNewConnections();
struct qsocket_s *NET_Connect(const char *host);
bool NET_CanSendMessage(struct qsocket_s *sock);
int NET_GetMessage(struct qsocket_s *sock);
int NET_SendMessage(struct qsocket_s *sock, sizebuf_t *data);
int NET_SendUnreliableMessage(struct qsocket_s *sock, sizebuf_t *data);
int NET_SendToAll(sizebuf_t *data, double blocktime);
void NET_Close(struct qsocket_s *sock);
void NET_Poll();
void NET_Slist_f();

extern dprograms_t *progs;                                            // progs.h
extern dfunction_t *pr_functions;
extern dstatement_t *pr_statements;
extern globalvars_t *pr_global_struct;
extern float *pr_globals; // same as pr_global_struct
extern int pr_edict_size; // in bytes
void PR_Init (void);
void PR_ExecuteProgram (func_t fnum);
void PR_LoadProgs (void);
const char *PR_GetString (int num);
int PR_SetEngineString (const char *s);
int PR_AllocString (int bufferlength, char **ptr);
void PR_Profile_f (void);
edict_t *ED_Alloc (void);
void ED_Free (edict_t *ed);
void ED_Print (edict_t *ed);
void ED_Write (FILE *f, edict_t *ed);
const char *ED_ParseEdict (const char *data, edict_t *ent);
void ED_WriteGlobals (FILE *f);
const char *ED_ParseGlobals (const char *data);
void ED_LoadFromFile (const char *data);
edict_t *EDICT_NUM(int);
int NUM_FOR_EDICT(edict_t*);
extern const int type_size[NUM_TYPE_SIZES];
typedef void (*builtin_t) (void);
extern const builtin_t *pr_builtins;
extern const int pr_numbuiltins;
extern int pr_argc;
extern bool pr_trace;
extern dfunction_t *pr_xfunction;
extern int pr_xstatement;
extern unsigned short pr_crc;
void PR_RunError (const char *error, ...);
void ED_PrintEdicts (void);
void ED_PrintNum (int ent);
eval_t *GetEdictFieldValue(edict_t *ed, const char *field);
#undef EX
#endif
