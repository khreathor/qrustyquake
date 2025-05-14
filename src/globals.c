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

vec3_t chase_pos;                                                     // chase.c
vec3_t chase_angles;
vec3_t chase_dest;
vec3_t chase_dest_angles;

kbutton_t in_mlook, in_klook;                                      // cl_input.c
kbutton_t in_left, in_right, in_forward, in_back;
kbutton_t in_lookup, in_lookdown, in_moveleft, in_moveright;
kbutton_t in_strafe, in_speed, in_use, in_jump, in_attack;
kbutton_t in_up, in_down;

f32 v_dmg_time, v_dmg_roll, v_dmg_pitch;                               // view.c
vec3_t forward, right, up;
cshift_t cshift_empty = { { 130, 80, 50 }, 0 }; // Palette flashes 
cshift_t cshift_water = { { 130, 80, 50 }, 128 };
cshift_t cshift_slime = { { 0, 25, 5 }, 150 };
cshift_t cshift_lava = { { 255, 80, 0 }, 150 };
u8 gammatable[256]; // palette is sent through this
s32 in_impulse;

edict_t *sv_player;                                                 // sv_user.c
f32 *angles;
f32 *origin;
f32 *velocity;
bool onground;
usercmd_t cmd;

client_static_t cls;                                                // cl_main.c
client_state_t cl;
efrag_t cl_efrags[MAX_EFRAGS];
entity_t cl_entities[MAX_EDICTS];
entity_t cl_static_entities[MAX_STATIC_ENTITIES];
lightstyle_t cl_lightstyle[MAX_LIGHTSTYLES];
dlight_t cl_dlights[MAX_DLIGHTS];
s32 cl_numvisedicts;
entity_t *cl_visedicts[MAX_VISEDICTS];

s32 bitcounts[16];                                                 // cl_parse.c
s8 *svc_strings[] = {
	"svc_bad",
	"svc_nop",
	"svc_disconnect",
	"svc_updatestat",
	"svc_version", // [s64] server version
	"svc_setview", // [s16] entity number
	"svc_sound", // <see code>
	"svc_time", // [f32] server time
	"svc_print", // [string] null terminated string
	"svc_stufftext", // [string] stuffed into client's console buffer
			 // the string should be \n terminated
	"svc_setangle", // [vec3] set the view angle to this absolute value
	"svc_serverinfo", // [s64] version
			  // [string] signon string
			  // [string]..[0]model cache [string]...[0]sounds cache
			  // [string]..[0]item cache
	"svc_lightstyle", // [u8] [string]
	"svc_updatename", // [u8] [string]
	"svc_updatefrags", // [u8] [s16]
	"svc_clientdata", // <shortbits + data>
	"svc_stopsound", // <see code>
	"svc_updatecolors", // [u8] [u8]
	"svc_particle", // [vec3] <variable>
	"svc_damage", // [u8] impact [u8] blood [vec3] from
	"svc_spawnstatic",
	"OBSOLETE svc_spawnbinary",
	"svc_spawnbaseline",
	"svc_temp_entity", // <variable>
	"svc_setpause",
	"svc_signonnum",
	"svc_centerprint",
	"svc_killedmonster",
	"svc_foundsecret",
	"svc_spawnstaticsound",
	"svc_intermission",
	"svc_finale", // [string] music [string] text
	"svc_cdtrack", // [u8] track [u8] looptrack
	"svc_sellscreen",
	"svc_cutscene"
};

s32 num_temp_entities;                                              // cl_tent.c
entity_t cl_temp_entities[MAX_TEMP_ENTITIES];
beam_t cl_beams[MAX_BEAMS];
sfx_t *cl_sfx_wizhit;
sfx_t *cl_sfx_knighthit;
sfx_t *cl_sfx_tink1;
sfx_t *cl_sfx_ric1;
sfx_t *cl_sfx_ric2;
sfx_t *cl_sfx_ric3;
sfx_t *cl_sfx_r_exp3;

cmdalias_t *cmd_alias;                                                  // cmd.c
s32 trashtest;
s32 *trashspot;
bool cmd_wait;
sizebuf_t cmd_text;
cmd_source_t cmd_source;
cmd_function_t *cmd_functions; // possible commands to execute

s32 safemode;                                                        // common.c
bool fitzmode;
s8 com_token[1024];
s32 com_argc;
s8 **com_argv;
s8 com_cmdline[CMDLINE_LENGTH];
bool standard_quake = true, rogue, hipnotic;
bool host_bigendian;
s32 msg_readcount;
bool msg_badread;
s32 com_filesize;
s8 com_gamedir[MAX_OSPATH];
s8 com_basedir[MAX_OSPATH];
s32 file_from_pak;
searchpath_t *com_searchpaths;
searchpath_t *com_base_searchpaths;
s16 (*BigShort) (s16 l);
s16 (*LittleShort) (s16 l);
s32 (*BigLong) (s32 l);
s32 (*LittleLong) (s32 l);
f32 (*BigFloat) (f32 l);
f32 (*LittleFloat) (f32 l);

bool con_forcedup; // because no entities to refresh                // console.c
s32 con_totallines; // total lines in console scrollback
s32 con_backscroll; // lines up from bottom to display
s32 con_current; // where next message will be printed
s32 con_x; // offset in current line for next print
s8 *con_text = 0;
s32 con_linewidth;
f32 con_cursorspeed = 4;
s32 con_vislines;
bool con_debuglog;
bool con_initialized;
s32 con_notifylines; // scan lines to clear for notify lines
f32 con_times[NUM_CON_TIMES];

f32 scale_for_mip;                                                   // d_edge.c
s32 ubasestep, errorterm, erroradjustup, erroradjustdown;
s32 vstartscan;
vec3_t transformed_modelorg;

surfcache_t *d_initial_rover;                                        // d_init.c
bool d_roverwrapped;
s32 d_minmip;
f32 d_scalemip[NUM_MIPS - 1];

s32 d_y_aspect_shift, d_pix_min, d_pix_max, d_pix_shift;           // d_modech.c
s32 d_vrectx, d_vrecty, d_vrectright_particle, d_vrectbottom_particle;
s32 d_scantable[MAXHEIGHT];
s16 *zspantable[MAXHEIGHT];

u32 sb_updates; // if >= vid.numpages, no update needed                // sbar.c

sspan_t spans[MAXHEIGHT + 1];                                      // d_sprite.c

f32 surfscale;                                                       // d_surf.c
bool r_cache_thrash; // set if surface cache is thrashing
u64 sc_size;
surfcache_t *sc_rover, *sc_base;
s32 lmonly; // render lightmap only, for lit water

cachepic_t menu_cachepics[MAX_CACHED_PICS];                            // draw.c
s32 menu_numcachepics;
u8 *draw_chars; // 8*8 graphic characters
qpic_t *draw_disc;
qpic_t *draw_backtile;
s32 drawlayer = 0;

quakeparms_t host_parms;                                               // host.c
bool host_initialized; // true if into command execution
bool isDedicated;
f64 host_frametime;
f64 host_rawframetime;
f32 host_netinterval;
f64 host_time;
f64 realtime; // without any filtering or bounding
f64 oldrealtime; // last frame run
s32 host_framecount;
s32 host_hunklevel;
s32 minimum_memory;
client_t *host_client; // current client
jmp_buf host_abortserver;
u8 *host_basepal;
u8 *host_colormap;

s32 current_skill;                                                 // host_cmd.c
bool noclip_anglehack;

s8 key_lines[32][MAXCMDLINE];                                          // keys.c
s32 key_linepos;
s32 key_lastpress;
s32 edit_line = 0;
keydest_t key_dest;
s32 key_count; // incremented every key event
s8 *keybindings[256];
s32 key_repeats[256]; // if > 1, it is autorepeating
s8 chat_buffer[32];
bool team_message = false;

f32 cur_ent_alpha = 1;                                             // d_polyse.c

vec3_t lightcolor; //johnfitz -- lit support via lordhavoc          // r_light.c

s32 dither_pat = 0;                                                  // d_scan.c
s32 lwmark = 0;
u8 *litwater_base;
