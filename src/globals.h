#include "quakedef.h"

#ifndef QGLOBALS_
#define QGLOBALS_

#define EX extern // globals.h only
extern f64 host_time;                                             // quakedef
extern bool noclip_anglehack;
extern quakeparms_t host_parms;
extern bool host_initialized; // true if into command execution
extern f64 host_frametime;
extern u8 *host_basepal;
extern u8 *host_colormap;
extern bool isDedicated;
extern s32 minimum_memory;
extern s32 host_framecount; // incremented every frame, never reset
extern f64 realtime; //  never reset
extern bool msg_suppress_1;
extern s32 current_skill;
void Host_ClearMemory();
void Host_ServerFrame();
void Host_InitCommands();
void Host_Init();
void Host_Shutdown();
void Host_Error(s8 *error, ...);
void Host_EndGame(s8 *message, ...);
void Host_Frame(f32 time);
void Host_Quit_f();
void Host_ClientCommands(s8 *fmt, ...);
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
EX u8 rgb_lut[RGB_LUT_SIZE];
EX s32 rgb_lut_built;
EX void init_color_conv();
EX u8 rgbtoi_lab(u8 r, u8 g, u8 b);
EX u8 rgbtoi(u8 r, u8 g, u8 b);
EX void build_color_mix_lut(cvar_t *cvar);
EX void R_BuildLitLUT();
EX u32 lfsr_random();

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

EX u8 r_foundtranswater, r_wateralphapass;                         // r_main.c
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
EX u8 *r_warpbuffer;
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

EX u8 gammatable[256]; // palette is sent through this               // view.h
EX u8 ramps[3][256];
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
EX void CRC_ProcessByte(u16 *crcvalue, u8 data);
EX u16 CRC_Value(u16 crcvalue);
EX u16 CRC_Block (const u8 *start, s32 count); //johnfitz -- texture crc

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
qsocket_t *Loop_Connect (const s8 *host);
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
EX s32 net_numsockets;
EX net_landriver_t net_landrivers[];
EX const s32 net_numlandrivers;
EX net_driver_t net_drivers[];
EX const s32 net_numdrivers;
EX s32 net_driverlevel;
EX s32 messagesSent;
EX s32 messagesReceived;
EX s32 unreliableMessagesSent;
EX s32 unreliableMessagesReceived;
EX s32 hostCacheCount;
EX hostcache_t hostcache[HOSTCACHESIZE];
qsocket_t *NET_NewQSocket ();
void NET_FreeQSocket(qsocket_t *);
f64 SetNetTime();
void SchedulePollProcedure(PollProcedure *pp, f64 timeOffset);

extern s32 con_totallines;                                          // console.h
extern s32 con_backscroll;
extern bool con_forcedup; // because no entities to refresh
extern bool con_initialized;
extern u8 *con_chars;
extern s32 con_notifylines; // scan lines to clear for notify lines
void Con_DrawCharacter (s32 cx, s32 line, s32 num);
void Con_CheckResize (void);
void Con_Init (void);
void Con_DrawConsole (s32 lines, bool drawinput);
void Con_Print (s8 *txt);
void Con_Printf (const s8 *fmt, ...);
void Con_DPrintf (s8 *fmt, ...);
void Con_SafePrintf (s8 *fmt, ...);
void Con_Clear_f (void);
void Con_DrawNotify (void);
void Con_ClearNotify (void);
void Con_ToggleConsole_f (void);

s32 Datagram_Init ();                                              // net_dgrm.h
void Datagram_Listen (bool state);
void Datagram_SearchForHosts (bool xmit);
qsocket_t *Datagram_Connect (const s8 *host);
qsocket_t *Datagram_CheckNewConnections ();
s32 Datagram_GetMessage (qsocket_t *sock);
s32 Datagram_SendMessage (qsocket_t *sock, sizebuf_t *data);
s32 Datagram_SendUnreliableMessage (qsocket_t *sock, sizebuf_t *data);
bool Datagram_CanSendMessage (qsocket_t *sock);
bool Datagram_CanSendUnreliableMessage (qsocket_t *sock);
void Datagram_Close (qsocket_t *sock);
void Datagram_Shutdown ();

extern qpic_t *draw_disc; // also used on sbar                         // draw.h
void Draw_Init ();
void Draw_CharacterScaled (s32 x, s32 y, s32 num, s32 scale);
void Draw_PicScaled (s32 x, s32 y, qpic_t *pic, s32 scale);
void Draw_PicScaledPartial(s32 x,s32 y,s32 l,s32 t,s32 w,s32 h,qpic_t *p,s32 s);
void Draw_TransPicScaled (s32 x, s32 y, qpic_t *pic, s32 scale);
void Draw_TransPicTranslateScaled(s32 x, s32 y, qpic_t *p, u8 *tl, s32 scale);
void Draw_ConsoleBackground (s32 lines);
void Draw_TileClear (s32 x, s32 y, s32 w, s32 h);
void Draw_Fill (s32 x, s32 y, s32 w, s32 h, s32 c);
void Draw_FadeScreen ();
void Draw_StringScaled (s32 x, s32 y, s8 *str, s32 scale);
qpic_t *Draw_PicFromWad (s8 *name);
qpic_t *Draw_CachePic (s8 *path);

void Mod_Init ();                                          // model.h
void Mod_ClearAll ();
void Mod_ResetAll (); // for gamedir changes (Host_Game_f)
model_t *Mod_ForName (const s8 *name, bool crash);
void *Mod_Extradata (model_t *mod); // handles caching
void Mod_TouchModel (const s8 *name);
mleaf_t *Mod_PointInLeaf (vec3_t p, model_t *model);
u8 *Mod_LeafPVS (mleaf_t *leaf, model_t *model);
u8 *Mod_NoVisPVS (model_t *model);
void Mod_SetExtraFlags (model_t *mod);

void SCR_Init ();                                                    // screen.h
void SCR_UpdateScreen ();
void SCR_SizeUp ();
void SCR_SizeDown ();
void SCR_CenterPrint (const s8 *str);
void SCR_BeginLoadingPlaque ();
void SCR_EndLoadingPlaque ();
s32 SCR_ModalMessage (s8 *text);
extern f32 scr_con_current;
extern f32 scr_conlines; // lines of console to display
extern s32 sb_lines;
extern u32 scr_fullupdate; // set to 0 to force full redraw
extern u32 clearnotify; // set to 0 whenever notify text is drawn
extern bool scr_disabled_for_loading;
extern bool scr_skipupdate;
extern bool block_drawing;
extern hudstyle_t hudstyle;

sys_socket_t UDP_Init();                                            // net_udp.h
void UDP_Shutdown();
void UDP_Listen(bool state);
sys_socket_t UDP_OpenSocket(s32 port);
s32 UDP_CloseSocket(sys_socket_t socketid);
s32 UDP_Connect(sys_socket_t socketid, struct qsockaddr *addr);
sys_socket_t UDP_CheckNewConnections();
s32 UDP_Read(sys_socket_t socketid, u8 *buf, s32 len, struct qsockaddr *addr);
s32 UDP_Write(sys_socket_t socketid, u8 *buf, s32 len, struct qsockaddr *addr);
s32 UDP_Broadcast(sys_socket_t socketid, u8 *buf, s32 len);
const s8 *UDP_AddrToString(struct qsockaddr *addr);
s32 UDP_StringToAddr(const s8 *string, struct qsockaddr *addr);
s32 UDP_GetSocketAddr(sys_socket_t socketid, struct qsockaddr *addr);
s32 UDP_GetNameFromAddr(struct qsockaddr *addr, s8 *name);
s32 UDP_GetAddrFromName(const s8 *name, struct qsockaddr *addr);
s32 UDP_AddrCompare(struct qsockaddr *addr1, struct qsockaddr *addr2);
s32 UDP_GetSocketPort(struct qsockaddr *addr);
s32 UDP_SetSocketPort(struct qsockaddr *addr, s32 port);

extern s32 wad_numlumps;                                                // wad.h
extern lumpinfo_t *wad_lumps;
extern u8 *wad_base;
void W_LoadWadFile (void); //johnfitz -- filename is now hard-coded
void W_CleanupName (const s8 *in, s8 *out);
void *W_GetLumpName (const s8 *name);
void *W_GetLumpNum (s32 num);
wad_t *W_LoadWadList (const s8 *names);
void W_FreeWadList (wad_t *wads);
lumpinfo_t *W_GetLumpinfoList (wad_t *wads, const s8 *name, wad_t **out_wad);
void SwapPic (qpic_t *pic);

extern s32 reinit_surfcache;                                         // render.h
extern vec3_t r_origin, vpn, vright, vup;
extern s32 reinit_surfcache;
extern bool r_cache_thrash;
void R_Init();
void R_InitTextures();
void R_InitEfrags();
void R_RenderView();
void R_ViewChanged(vrect_t *pvrect, s32 lineadj, f32 aspect);
void R_InitSky(struct texture_s *mt);
void R_AddEfrags(entity_t *ent);
void R_RemoveEfrags(entity_t *ent);
void R_NewMap();
void R_ParseParticleEffect();
void R_RunParticleEffect(vec3_t org, vec3_t dir, s32 color, s32 count);
void R_RocketTrail(vec3_t start, vec3_t end, s32 type);
void R_EntityParticles(entity_t *ent);
void R_BlobExplosion(vec3_t org);
void R_ParticleExplosion(vec3_t org);
void R_ParticleExplosion2(vec3_t org, s32 colorStart, s32 colorLength);
void R_LavaSplash(vec3_t org);
void R_TeleportSplash(vec3_t org);
void R_PushDlights();
s32 D_SurfaceCacheForRes(s32 width, s32 height);
void D_FlushCaches();
void D_DeleteSurfaceCache();
void D_InitCaches(void *buffer, s32 size);
void R_SetVrect(vrect_t *pvrect, vrect_t *pvrectin, s32 lineadj);

extern s32 d_spanpixcount;                                          // d_iface.h
extern f32 r_aliasuvscale;
extern s32 r_pixbytes;
extern affinetridesc_t r_affinetridesc;
extern spritedesc_t r_spritedesc;
extern zpointdesc_t r_zpointdesc;
extern polydesc_t r_polydesc;
extern s32 d_con_indirect;
extern vec3_t r_pright, r_pup, r_ppn;
extern void *acolormap;
extern drawsurf_t r_drawsurf;
extern f32 skyspeed, skyspeed2;
extern f32 skytime;
extern s32 c_surf;
extern vrect_t scr_vrect;
extern u8 *r_warpbuffer;
extern u8 *r_skysource;
void D_BeginDirectRect(s32 x, s32 y, u8 *pbitmap, s32 width, s32 height);
void D_PolysetDraw();
void D_PolysetDrawFinalVerts(finalvert_t *fv, s32 numverts);
void D_DrawParticle(particle_t *pparticle);
void D_DrawPoly();
void D_DrawSprite();
void D_DrawSurfaces();
void D_Init();
void D_ViewChanged();
void D_SetupFrame();
void D_TurnZOn();
void D_WarpScreen();
void D_FillRect(vrect_t *vrect, s32 color);
void D_DrawRect();
void R_DrawSurface();
void D_PolysetUpdateTables();

extern vec3_t vec3_origin;                                          // mathlib.h
extern s32 nanmask;
void VectorMA(vec3_t veca, f32 scale, vec3_t vecb, vec3_t vecc);
vec_t Length(vec3_t v);
void CrossProduct(vec3_t v1, vec3_t v2, vec3_t cross);
f32 VectorNormalize(vec3_t v); // returns vector length
void VectorInverse(vec3_t v);
void VectorScale(vec3_t in, vec_t scale, vec3_t out);
void R_ConcatRotations(f32 in1[3][3], f32 in2[3][3], f32 out[3][3]);
void R_ConcatTransforms(f32 in1[3][4], f32 in2[3][4], f32 out[3][4]);
void FloorDivMod(f64 numer, f64 denom, s32 *quotient, s32 *rem);
s32 GreatestCommonDivisor(s32 i1, s32 i2);
void AngleVectors(vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
s32 BoxOnPlaneSide(vec3_t emins, vec3_t emaxs, struct mplane_s *plane);
f32 anglemod(f32 a);
vec_t VectorLength(vec3_t v);
void VectorSubtract(const vec3_t a, const vec3_t b, vec3_t c);
void VectorAdd(const vec3_t a, const vec3_t b, vec3_t c);
void VectorCopy(const vec3_t a, vec3_t b);

extern s32 cachewidth;                                             // r_shared.h
extern u8 *cacheblock;
extern s32 screenwidth;
extern f32 pixelAspect;
extern s32 r_drawnpolycount;
extern s32 sintable[SIN_BUFFER_SIZE];
extern s32 intsintable[SIN_BUFFER_SIZE];
extern vec3_t vup, base_vup;
extern vec3_t vpn, base_vpn;
extern vec3_t vright, base_vright;
extern entity_t *currententity;
extern vec3_t sxformaxis[4]; // s axis transformed into viewspace
extern vec3_t txformaxis[4]; // t axis transformed into viewspac
extern vec3_t modelorg, base_modelorg;
extern f32 xcenter, ycenter;
extern f32 xscale, yscale;
extern f32 xscaleinv, yscaleinv;
extern f32 xscaleshrink, yscaleshrink;
extern s32 d_lightstylevalue[256]; // 8.8 frac of base light value
extern s32 r_skymade;
extern s8 skybox_name[1024];
extern s32 ubasestep, errorterm, erroradjustup, erroradjustdown;
extern surf_t *surfaces, *surface_p, *surf_max;
extern void TransformVector(vec3_t in, vec3_t out);
extern void SetUpForLineScan(s32 startvertu, s32 startvertv,
		s32 endvertu, s32 endvertv);
extern void R_MakeSky();
void Sky_LoadSkyBox (const s8 *name);
extern void R_DrawLine(polyvert_t *polyvert0, polyvert_t *polyvert1);

void SV_ClearWorld();                                                 // world.h
void SV_UnlinkEdict(edict_t *ent);
void SV_LinkEdict(edict_t *ent, bool touch_triggers);
s32 SV_PointContents(vec3_t p);
s32 SV_TruePointContents(vec3_t p);
edict_t *SV_TestEntityPosition(edict_t *ent);
trace_t SV_Move(vec3_t start, vec3_t mins, vec3_t maxs,
		vec3_t end, s32 type, edict_t *passedict);

extern f32 scale_for_mip;                                         // d_local.h
extern bool d_roverwrapped;
extern surfcache_t *sc_rover;
extern surfcache_t *d_initial_rover;
extern f32 d_sdivzstepu, d_tdivzstepu, d_zistepu;
extern f32 d_sdivzstepv, d_tdivzstepv, d_zistepv;
extern f32 d_sdivzorigin, d_tdivzorigin, d_ziorigin;
extern s32 sadjust, tadjust;
extern s32 bbextents, bbextentt;
extern s16 *d_pzbuffer;
extern u32 d_zrowbytes, d_zwidth;
extern s32 *d_pscantable;
extern s32 d_scantable[MAXHEIGHT];
extern s32 d_vrectx, d_vrecty, d_vrectright_particle, d_vrectbottom_particle;
extern s32 d_y_aspect_shift, d_pix_min, d_pix_max, d_pix_shift;
extern u8 *d_viewbuffer;
extern s16 *zspantable[MAXHEIGHT];
extern s32 d_minmip;
extern f32 d_scalemip[3];
extern s32 r_pass;
extern f32 winquake_surface_liquid_alpha;
extern u8 color_mix_lut[256][256][FOG_LUT_LEVELS];
extern u8 fog_pal_index;
extern s32 lmonly;
extern u8 *litwater_base;
extern s32 lwmark;
void D_DrawSpans8(espan_t *pspans);
void D_DrawTransSpans8(espan_t *pspans, f32 opacity);
void D_DrawZSpans(espan_t *pspans);
void Turbulent8(espan_t *pspan, f32 opacity);
void D_SpriteDrawSpans(sspan_t *pspan);
void D_DrawSkyScans8(espan_t *pspan);
void D_DrawSkyboxScans8(espan_t *pspans);
void R_ShowSubDiv();
extern void(*prealspandrawer)();
surfcache_t *D_CacheSurface(msurface_t *surface, s32 miplevel);
extern s32 D_MipLevelForScale(f32 scale);
extern s32 dither_pat;
extern s32 D_Dither(u8 *pos);

extern s32 DEFAULTnet_hostport;                                         // net.h
extern s32 net_hostport;
extern f64 net_time;
extern sizebuf_t net_message;
extern s32 net_activeconnections;
extern bool slistInProgress;
extern bool slistSilent;
extern bool slistLocal;
extern s32 hostCacheCount;
extern bool tcpipAvailable;
extern s8 my_tcpip_address[NET_NAMELEN];
void NET_Init();
void NET_Shutdown();
struct qsocket_s *NET_CheckNewConnections();
struct qsocket_s *NET_Connect(const s8 *host);
bool NET_CanSendMessage(struct qsocket_s *sock);
s32 NET_GetMessage(struct qsocket_s *sock);
s32 NET_SendMessage(struct qsocket_s *sock, sizebuf_t *data);
s32 NET_SendUnreliableMessage(struct qsocket_s *sock, sizebuf_t *data);
s32 NET_SendToAll(sizebuf_t *data, f64 blocktime);
void NET_Close(struct qsocket_s *sock);
void NET_Poll();
void NET_Slist_f();

extern dprograms_t *progs;                                            // progs.h
extern dfunction_t *pr_functions;
extern dstatement_t *pr_statements;
extern globalvars_t *pr_global_struct;
extern f32 *pr_globals; // same as pr_global_struct
extern s32 pr_edict_size; // in bytes
void PR_Init (void);
void PR_ExecuteProgram (func_t fnum);
void PR_LoadProgs (void);
const s8 *PR_GetString (s32 num);
s32 PR_SetEngineString (const s8 *s);
s32 PR_AllocString (s32 bufferlength, s8 **ptr);
void PR_Profile_f (void);
edict_t *ED_Alloc (void);
void ED_Free (edict_t *ed);
void ED_Print (edict_t *ed);
void ED_Write (FILE *f, edict_t *ed);
const s8 *ED_ParseEdict (const s8 *data, edict_t *ent);
void ED_WriteGlobals (FILE *f);
const s8 *ED_ParseGlobals (const s8 *data);
void ED_LoadFromFile (const s8 *data);
edict_t *EDICT_NUM(s32);
s32 NUM_FOR_EDICT(edict_t*);
extern const s32 type_size[NUM_TYPE_SIZES];
extern const builtin_t *pr_builtins;
extern const s32 pr_numbuiltins;
extern s32 pr_argc;
extern bool pr_trace;
extern dfunction_t *pr_xfunction;
extern s32 pr_xstatement;
extern u16 pr_crc;
void PR_RunError (const s8 *error, ...);
void ED_PrintEdicts (void);
void ED_PrintNum (s32 ent);
eval_t *GetEdictFieldValue(edict_t *ed, const s8 *field);

void Cvar_RegisterVariable (cvar_t *variable);                         // cvar.h
void Cvar_SetCallback (cvar_t *var, cvarcallback_t func);
void Cvar_Set (const s8 *var_name, const s8 *value);
void Cvar_SetValue (const s8 *var_name, const f32 value);
void Cvar_SetROM (const s8 *var_name, const s8 *value);
void Cvar_SetValueROM (const s8 *var_name, const f32 value);
void Cvar_SetQuick (cvar_t *var, const s8 *value);
void Cvar_SetValueQuick (cvar_t *var, const f32 value);
f32 Cvar_VariableValue (const s8 *var_name);
const s8 *Cvar_VariableString (const s8 *var_name);
bool Cvar_Command (void);
void Cvar_WriteVariables (FILE *f);
cvar_t *Cvar_FindVar (const s8 *var_name);
cvar_t *Cvar_FindVarAfter (const s8 *prev_name, u32 with_flags);
void Cvar_LockVar (const s8 *var_name);
void Cvar_UnlockVar (const s8 *var_name);
void Cvar_UnlockAll (void);
void Cvar_Init (void);
const s8 *Cvar_CompleteVariable (const s8 *partial);

void Memory_Init (void *buf, s32 size);                                // zone.h
void Z_Free (void *ptr);
void *Z_Malloc (s32 size); // returns 0 filled memory
void *Z_Realloc (void *ptr, s32 size);
s8 *Z_Strdup (const s8 *s);
void *Hunk_Alloc (s32 size); // returns 0 filled memory
void *Hunk_AllocName (s32 size, const s8 *name);
void *Hunk_HighAllocName (s32 size, const s8 *name);
s8 *Hunk_Strdup (const s8 *s, const s8 *name);
s32 Hunk_LowMark (void);
void Hunk_FreeToLowMark (s32 mark);
s32 Hunk_HighMark (void);
void Hunk_FreeToHighMark (s32 mark);
void *Hunk_TempAlloc (s32 size);
void Hunk_Check (void);
void Cache_Flush (void);
void *Cache_Check (cache_user_t *c);
void Cache_Free (cache_user_t *c, bool freetextures);
void *Cache_Alloc (cache_user_t *c, s32 size, const s8 *name);
void Cache_Report (void);

extern cmd_source_t cmd_source;                                         // cmd.h
void Cmd_Init ();
void Cbuf_Init ();
void Cbuf_AddText (const s8 *text);
void Cbuf_InsertText (s8 *text);
void Cbuf_Execute ();
void Cmd_AddCommand (s8 *cmd_name, xcommand_t function);
bool Cmd_Exists (const s8 *cmd_name);
s8 *Cmd_CompleteCommand (s8 *partial);
s32 Cmd_Argc ();
s8 *Cmd_Argv (s32 arg);
s8 *Cmd_Args ();
void Cmd_TokenizeString (const s8 *text);
void Cmd_ExecuteString (const s8 *text, cmd_source_t src);
void Cmd_ForwardToServer ();
void Cmd_Print (s8 *text);

void S_Init (void);                                                 // q_sound.h
void S_Startup (void);
void S_Shutdown (void);
void S_StartSound (s32 entnum, s32 entchannel, sfx_t *sfx, vec3_t origin, f32 fvol, f32 attenuation);
void S_StaticSound (sfx_t *sfx, vec3_t origin, f32 vol, f32 attenuation);
void S_StopSound (s32 entnum, s32 entchannel);
void S_StopAllSounds(bool clear);
void S_ClearBuffer (void);
void S_Update (vec3_t origin, vec3_t forward, vec3_t right, vec3_t up);
void S_ExtraUpdate (void);
void S_BlockSound (void);
void S_UnblockSound (void);
sfx_t *S_PrecacheSound (const s8 *sample);
void S_TouchSound (const s8 *sample);
void S_ClearPrecache (void);
void S_BeginPrecaching (void);
void S_EndPrecaching (void);
void S_PaintChannels (s32 endtime);
void S_InitPaintChannels (void);
channel_t *SND_PickChannel (s32 entnum, s32 entchannel);
void SND_Spatialize (channel_t *ch);
void S_RawSamples(s32 samples, s32 rate, s32 width, s32 channels, u8 * data, f32 volume);
bool SNDDMA_Init(dma_t *dma);
s32 SNDDMA_GetDMAPos(void);
void SNDDMA_Shutdown(void);
void SNDDMA_LockBuffer(void);
void SNDDMA_Submit(void);
void SNDDMA_BlockSound(void);
void SNDDMA_UnblockSound(void);
extern channel_t snd_channels[MAX_CHANNELS];
extern volatile dma_t *shm;
extern s32 total_channels;
extern s32 soundtime;
extern s32 paintedtime;
extern s32 s_rawend;
extern vec3_t listener_origin;
extern vec3_t listener_forward;
extern vec3_t listener_right;
extern vec3_t listener_up;
extern portable_samplepair_t s_rawsamples[MAX_RAW_SAMPLES];
void S_LocalSound (const s8 *name);
sfxcache_t *S_LoadSound (sfx_t *s);
wavinfo_t GetWavinfo (const s8 *name, u8 *wav, s32 wavlength);
void SND_InitScaletable (void);

extern clipplane_t view_clipplanes[4];                              // r_local.h
extern mplane_t screenedge[4];
extern vec3_t r_origin;
extern vec3_t r_entorigin;
extern f32 screenAspect;
extern f32 verticalFieldOfView;
extern f32 xOrigin, yOrigin;
extern u32 r_visframecount;
extern s32 vstartscan;
extern bool insubmodel; // current entity info
extern vec3_t r_worldmodelorg; // current entity info
extern s32 c_faceclip;
extern s32 r_polycount;
extern s32 r_wholepolycount;
extern model_t *cl_worldmodel;
extern s32 *pfrustum_indexes[4];
extern s32 ubasestep, errorterm, erroradjustup, erroradjustdown;
extern s32 vstartscan;
extern s32 sadjust, tadjust;
extern s32 bbextents, bbextentt;
extern mvertex_t *r_ptverts, *r_ptvertsmax;
extern vec3_t sbaseaxis[3], tbaseaxis[3];
extern f32 entity_rotation[3][3];
extern s32 reinit_surfcache;
extern s32 r_currentkey;
extern s32 r_currentbkey;
extern s32 numbtofpolys;
extern btofpoly_t *pbtofpolys;
extern s32 numverts;
extern s32 a_skinwidth;
extern mtriangle_t *ptriangles;
extern s32 numtriangles;
extern aliashdr_t *paliashdr;
extern mdl_t *pmdl;
extern f32 leftclip, topclip, rightclip, bottomclip;
extern s32 r_acliptype;
extern finalvert_t *pfinalverts;
extern auxvert_t *pauxverts;
extern s32 r_amodels_drawn;
extern edge_t *auxedges;
extern s32 r_numallocatededges;
extern edge_t *r_edges, *edge_p, *edge_max;
extern edge_t *newedges[MAXHEIGHT];
extern edge_t *removeedges[MAXHEIGHT];
extern s32 screenwidth;
extern edge_t edge_head; // FIXME: make stack vars when debugging done
extern edge_t edge_tail;
extern edge_t edge_aftertail;
extern s32 r_bmodelactive;
extern f32 aliasxscale, aliasyscale, aliasxcenter, aliasycenter;
extern f32 r_aliastransition, r_resfudge;
extern s32 r_outofsurfaces;
extern s32 r_outofedges;
extern mvertex_t *r_pcurrentvertbase;
extern s32 r_maxvalidedgeoffset;
extern f32 r_time1;
extern f32 dp_time1, dp_time2, db_time1, db_time2, rw_time1, rw_time2;
extern f32 se_time1, se_time2, de_time1, de_time2, dv_time1, dv_time2;
extern s32 r_frustum_indexes[4*6];
extern s32 r_maxsurfsseen, r_maxedgesseen, r_cnumsurfs;
extern bool r_surfsonstack;
extern cshift_t cshift_water;
extern bool r_dowarpold, r_viewchanged;
extern mleaf_t *r_viewleaf, *r_oldviewleaf;
extern vec3_t r_emins, r_emaxs;
extern mnode_t *r_pefragtopnode;
extern s32 r_clipflags;
extern u32 r_dlightframecount;
extern bool r_fov_greater_than_90;
extern s32 r_pass;
extern s32 color_lightmap;
extern s32 lmonly;
extern f32 map_fallbackalpha;
extern f32 map_wateralpha;
extern f32 map_lavaalpha;
extern f32 map_telealpha;
extern f32 map_slimealpha;
void R_DrawSprite();
void R_RenderFace(msurface_t *fa, s32 clipflags);
void R_RenderBmodelFace(bedge_t *pedges, msurface_t *psurf);
void R_TransformFrustum();
void R_SetSkyFrame();
void R_DrawSurfaceBlock16();
void R_DrawSurfaceBlock8();
texture_t *R_TextureAnimation(texture_t *base);
void R_GenSkyTile(void *pdest);
void R_GenSkyTile16(void *pdest);
void R_DrawSubmodelPolygons(model_t *pmodel, s32 clipflags);
void R_DrawSolidClippedSubmodelPolygons(model_t *pmodel);
void R_AddPolygonEdges(emitpoint_t *pverts, s32 numverts, s32 miplevel);
surf_t *R_GetSurf();
void R_AliasDrawModel(alight_t *plighting);
void R_BeginEdgeFrame();
void R_ScanEdges();
void D_DrawSurfaces();
void R_InsertNewEdges(edge_t *edgestoadd, edge_t *edgelist);
void R_StepActiveU(edge_t *pedge);
void R_RemoveEdges(edge_t *pedge);
extern void R_Surf8Start();
extern void R_Surf8End();
extern void R_Surf16Start();
extern void R_Surf16End();
extern void R_EdgeCodeStart();
extern void R_EdgeCodeEnd();
extern void R_RotateBmodel();
void R_InitTurb();
void R_ZDrawSubmodelPolys(model_t *clmodel);
bool R_AliasCheckBBox();
void R_DrawParticles();
void R_InitParticles();
void R_ClearParticles();
void R_ReadPointFile_f();
void R_AliasClipTriangle(mtriangle_t *ptri);
void R_RenderWorld();
void R_StoreEfrags(efrag_t **ppefrag);
void R_TimeRefresh_f();
void R_TimeGraph();
void R_PrintAliasStats();
void R_PrintTimes();
void R_PrintDSpeeds();
void R_AnimateLight();
s32 R_LightPoint(vec3_t p);
void R_SetupFrame();
void R_cshift_f();
void R_EmitEdge(mvertex_t *pv0, mvertex_t *pv1);
void R_ClipEdge(mvertex_t *pv0, mvertex_t *pv1, clipplane_t *clip);
void R_SplitEntityOnNode2(mnode_t *node);
void R_MarkLights(dlight_t *light, s32 bit, mnode_t *node);
u8 rgbtoi_lab(u8 r, u8 g, u8 b);
u8 rgbtoi(u8 r, u8 g, u8 b);
extern void init_color_conv();
extern void R_BuildLitLUT();
extern f32 R_WaterAlphaForTextureType(textype_t type);

extern client_state_t cl;                                            // client.h
extern efrag_t cl_efrags[MAX_EFRAGS];
extern entity_t cl_entities[MAX_EDICTS];
extern entity_t cl_static_entities[MAX_STATIC_ENTITIES];
extern lightstyle_t cl_lightstyle[MAX_LIGHTSTYLES];
extern dlight_t cl_dlights[MAX_DLIGHTS];
extern entity_t cl_temp_entities[MAX_TEMP_ENTITIES];
extern beam_t cl_beams[MAX_BEAMS];
dlight_t *CL_AllocDlight (s32 key);
void CL_DecayLights (void);
void CL_Init (void);
void CL_EstablishConnection (s8 *host);
void CL_Signon1 (void);
void CL_Signon2 (void);
void CL_Signon3 (void);
void CL_Signon4 (void);
void CL_Disconnect (void);
void CL_Disconnect_f (void);
void CL_NextDemo (void);
extern s32 cl_numvisedicts;
extern entity_t *cl_visedicts[MAX_VISEDICTS];
extern kbutton_t in_mlook, in_klook;
extern kbutton_t in_strafe;
extern kbutton_t in_speed;
void CL_InitInput (void);
void CL_SendCmd (void);
void CL_SendMove (const usercmd_t *cmd);
void CL_ParseTEnt (void);
void CL_UpdateTEnts (void);
void CL_ClearState (void);
s32 CL_ReadFromServer (void);
void CL_WriteToServer (usercmd_t *cmd);
void CL_BaseMove (usercmd_t *cmd);
f32 CL_KeyState (kbutton_t *key);
s8 *Key_KeynumToString (s32 keynum);
void CL_StopPlayback (void);
s32 CL_GetMessage (void);
void CL_Stop_f (void);
void CL_Record_f (void);
void CL_PlayDemo_f (void);
void CL_TimeDemo_f (void);
void CL_ParseServerMessage (void);
void CL_NewTranslation (s32 slot);
void V_StartPitchDrift (void);
void V_StopPitchDrift (void);
void V_RenderView (void);
void V_UpdatePalette (void);
void V_Register (void);
void V_ParseDamage (void);
void V_SetContentsColor (s32 contents);
void CL_InitTEnts (void);
void CL_SignonReply (void);

extern server_static_t svs; // persistant server info                // server.h
extern server_t sv; // local server
extern client_t *host_client;
extern edict_t *sv_player;
void SV_Init (void);
void SV_StartParticle (vec3_t org, vec3_t dir, s32 color, s32 count);
void SV_StartSound (edict_t *entity, s32 channel, const s8 *sample, s32 volume, f32 attenuation);
void SV_LocalSound (client_t *client, const s8 *sample);
void SV_DropClient (bool crash);
void SV_SendClientMessages (void);
void SV_ClearDatagram (void);
void SV_ReserveSignonSpace (s32 numbytes);
s32 SV_ModelIndex (const s8 *name);
void SV_SetIdealPitch (void);
void SV_AddUpdates (void);
void SV_ClientThink (void);
void SV_AddClientToServer (struct qsocket_s *ret);
void SV_ClientPrintf (const s8 *fmt, ...);
void SV_BroadcastPrintf (const s8 *fmt, ...);
void SV_Physics (void);
bool SV_CheckBottom (edict_t *ent);
bool SV_movestep (edict_t *ent, vec3_t move, bool relink);
void SV_WriteClientdataToMessage (edict_t *ent, sizebuf_t *msg);
void SV_MoveToGoal (void);
void SV_CheckForNewClients (void);
void SV_RunClients (void);
void SV_SaveSpawnparms (void);
void SV_SpawnServer (const s8 *server);

extern vec3_t chase_pos;                                              // chase.c
extern vec3_t chase_angles;
extern vec3_t chase_dest;
extern vec3_t chase_dest_angles;
extern void Cvar_RegisterVariable (cvar_t *variable);
extern bool SV_RecursiveHullCheck(hull_t *hull, s32 num, f32 p1f,
			f32 p2f, vec3_t p1, vec3_t p2, trace_t *trace);

EX kbutton_t in_mlook, in_klook;                                   // cl_input.c
EX kbutton_t in_left, in_right, in_forward, in_back;
EX kbutton_t in_lookup, in_lookdown, in_moveleft, in_moveright;
EX kbutton_t in_strafe, in_speed, in_use, in_jump, in_attack;
EX kbutton_t in_up, in_down;
EX s32 in_impulse;
EX void CL_AdjustAngles();
EX void IN_MLookDown();

EX f32 v_dmg_time, v_dmg_roll, v_dmg_pitch;                            // view.c
EX vec3_t forward, right, up;
EX cshift_t cshift_empty;
EX cshift_t cshift_water;
EX cshift_t cshift_slime;
EX cshift_t cshift_lava;
EX u8 gammatable[256];
EX s32 in_impulse;

EX edict_t *sv_player;                                              // sv_user.c
EX f32 *angles;
EX f32 *origin;
EX f32 *velocity;
EX bool onground;
EX usercmd_t cmd;

EX client_static_t cls;                                             // cl_main.c
EX client_state_t cl;
EX efrag_t cl_efrags[MAX_EFRAGS];
EX entity_t cl_entities[MAX_EDICTS];
EX entity_t cl_static_entities[MAX_STATIC_ENTITIES];
EX lightstyle_t cl_lightstyle[MAX_LIGHTSTYLES];
EX dlight_t cl_dlights[MAX_DLIGHTS];
EX s32 cl_numvisedicts;
EX entity_t *cl_visedicts[MAX_VISEDICTS];
EX void CL_AccumulateCmd();

EX s32 bitcounts[16];                                              // cl_parse.c
EX s8 *svc_strings[];

EX s32 num_temp_entities;                                           // cl_tent.c
EX entity_t cl_temp_entities[MAX_TEMP_ENTITIES];
EX beam_t cl_beams[MAX_BEAMS];
EX sfx_t *cl_sfx_wizhit;
EX sfx_t *cl_sfx_knighthit;
EX sfx_t *cl_sfx_tink1;
EX sfx_t *cl_sfx_ric1;
EX sfx_t *cl_sfx_ric2;
EX sfx_t *cl_sfx_ric3;
EX sfx_t *cl_sfx_r_exp3;

EX cmdalias_t *cmd_alias;                                               // cmd.c
EX s32 trashtest;
EX s32 *trashspot;
EX bool cmd_wait;
EX sizebuf_t cmd_text;
EX cmd_source_t cmd_source;
EX cmd_function_t *cmd_functions;
EX void Cbuf_Waited();

EX s32 safemode;                                                     // common.c
EX bool fitzmode;
EX s8 com_token[1024];
EX s32 com_argc;
EX s8 **com_argv;
EX s8 com_cmdline[CMDLINE_LENGTH];
EX bool standard_quake;
EX bool host_bigendian;
EX s32 msg_readcount;
EX bool msg_badread;
EX s32 com_filesize;
EX s8 com_gamedir[MAX_OSPATH];
EX s8 com_basedir[MAX_OSPATH];
EX s32 file_from_pak;
EX searchpath_t *com_searchpaths;
EX searchpath_t *com_base_searchpaths;
EX s16 (*BigShort) (s16 l);
EX s16 (*LittleShort) (s16 l);
EX s32 (*BigLong) (s32 l);
EX s32 (*LittleLong) (s32 l);
EX f32 (*BigFloat) (f32 l);
EX f32 (*LittleFloat) (f32 l);

EX bool con_forcedup; // because no entities to refresh             // console.c
EX s32 con_totallines; // total lines in console scrollback
EX s32 con_backscroll; // lines up from bottom to display
EX s32 con_current; // where next message will be printed
EX s32 con_x; // offset in current line for next print
EX s8 *con_text;
EX s32 con_linewidth;
EX f32 con_cursorspeed;
EX s32 con_vislines;
EX bool con_debuglog;
EX bool con_initialized;
EX s32 con_notifylines; // scan lines to clear for notify lines
EX f32 con_times[NUM_CON_TIMES];
EX s8 key_lines[32][MAXCMDLINE];
EX s32 edit_line;
EX s32 key_linepos;
EX bool team_message;
EX void M_Menu_Main_f();

EX f32 scale_for_mip;                                                // d_edge.c
EX s32 ubasestep, errorterm, erroradjustup, erroradjustdown;
EX s32 vstartscan;
EX vec3_t transformed_modelorg;

EX surfcache_t *d_initial_rover;                                     // d_init.c
EX bool d_roverwrapped;
EX s32 d_minmip;
EX f32 d_scalemip[NUM_MIPS - 1];

EX s32 d_y_aspect_shift, d_pix_min, d_pix_max, d_pix_shift;        // d_modech.c
EX s32 d_vrectx, d_vrecty, d_vrectright_particle, d_vrectbottom_particle;
EX s32 d_scantable[MAXHEIGHT];
EX s16 *zspantable[MAXHEIGHT];

EX u32 sb_updates; // if >= vid.numpages, no update needed             // sbar.c

EX sspan_t spans[MAXHEIGHT + 1];                                   // d_sprite.c

EX f32 surfscale;                                                    // d_surf.c
EX bool r_cache_thrash; // set if surface cache is thrashing
EX u64 sc_size;
EX surfcache_t *sc_rover, *sc_base;
EX s32 lmonly; // render lightmap only, for lit water

EX cachepic_t menu_cachepics[MAX_CACHED_PICS];                         // draw.c
EX s32 menu_numcachepics;
EX u8 *draw_chars; // 8*8 graphic characters
EX qpic_t *draw_disc;
EX qpic_t *draw_backtile;
EX s32 drawlayer;

EX quakeparms_t host_parms;                                            // host.c
EX bool host_initialized; // true if into command execution
EX bool isDedicated;
EX f64 host_frametime;
EX f64 host_rawframetime;
EX f32 host_netinterval;
EX f64 host_time;
EX f64 realtime; // without any filtering or bounding
EX f64 oldrealtime; // last frame run
EX s32 host_framecount;
EX s32 host_hunklevel;
EX s32 minimum_memory;
EX client_t *host_client; // current client
EX jmp_buf host_abortserver;
EX u8 *host_basepal;
EX u8 *host_colormap;

EX s32 current_skill;                                              // host_cmd.c
EX bool noclip_anglehack;
EX void M_Menu_Quit_f();

EX s8 key_lines[32][MAXCMDLINE];                                       // keys.c
EX s32 key_linepos;
EX s32 key_lastpress;
EX s32 edit_line;
EX keydest_t key_dest;
EX s32 key_count; // incremented every key event
EX s8 *keybindings[256];
EX s32 key_repeats[256]; // if > 1, it is autorepeating
EX s8 chat_buffer[32];
EX bool team_message;

EX f32 cur_ent_alpha;                                              // d_polyse.c

EX vec3_t lightcolor; //johnfitz -- lit support via lordhavoc       // r_light.c

EX s32 dither_pat;                                                   // d_scan.c
EX s32 lwmark;
EX u8 *litwater_base;
#undef EX
#endif
