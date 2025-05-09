#include "quakedef.h"                       // Default value |   | Save to 
                                            //               V   v config.cfg
cvar_t vid_mode=                               {"vid_mode", "0", 0, 0, 0, 0, 0};
cvar_t _vid_default_mode_win=     {"_vid_default_mode_win", "3", 1, 0, 0, 0, 0};
cvar_t scr_uiscale=                         {"scr_uiscale", "1", 1, 0, 0, 0, 0};
cvar_t sensitivityyscale=             {"sensitivityyscale", "1", 1, 0, 0, 0, 0};
cvar_t _windowed_mouse=                 {"_windowed_mouse", "0", 1, 0, 0, 0, 0};
cvar_t newoptions=                           {"newoptions", "1", 1, 0, 0, 0, 0};
cvar_t aspectr=                                 {"aspectr", "0", 1, 0, 0, 0, 0};
cvar_t realwidth=                             {"realwidth", "0", 0, 0, 0, 0, 0};
cvar_t realheight=                           {"realheight", "0", 0, 0, 0, 0, 0};
cvar_t r_draworder=                         {"r_draworder", "0", 0, 0, 0, 0, 0};
cvar_t r_speeds=                               {"r_speeds", "0", 0, 0, 0, 0, 0};
cvar_t r_timegraph=                         {"r_timegraph", "0", 0, 0, 0, 0, 0};
cvar_t r_graphheight=                    {"r_graphheight", "10", 0, 0, 0, 0, 0};
cvar_t r_clearcolor=                       {"r_clearcolor", "2", 0, 0, 0, 0, 0};
cvar_t r_waterwarp=                         {"r_waterwarp", "1", 0, 0, 0, 0, 0};
cvar_t r_fullbright=                       {"r_fullbright", "0", 0, 0, 0, 0, 0};
cvar_t r_drawentities=                   {"r_drawentities", "1", 0, 0, 0, 0, 0};
cvar_t r_drawviewmodel=                 {"r_drawviewmodel", "1", 0, 0, 0, 0, 0};
cvar_t r_aliasstats=                   {"r_polymodelstats", "0", 0, 0, 0, 0, 0};
cvar_t r_dspeeds=                             {"r_dspeeds", "0", 0, 0, 0, 0, 0};
cvar_t r_drawflat=                           {"r_drawflat", "0", 0, 0, 0, 0, 0};
cvar_t r_ambient=                             {"r_ambient", "0", 0, 0, 0, 0, 0};
cvar_t r_reportsurfout=                 {"r_reportsurfout", "0", 0, 0, 0, 0, 0};
cvar_t r_maxsurfs=                           {"r_maxsurfs", "0", 0, 0, 0, 0, 0};
cvar_t r_numsurfs=                           {"r_numsurfs", "0", 0, 0, 0, 0, 0};
cvar_t r_reportedgeout=                 {"r_reportedgeout", "0", 0, 0, 0, 0, 0};
cvar_t r_maxedges=                           {"r_maxedges", "0", 0, 0, 0, 0, 0};
cvar_t r_numedges=                           {"r_numedges", "0", 0, 0, 0, 0, 0};
cvar_t r_aliastransbase=             {"r_aliastransbase", "200", 0, 0, 0, 0, 0};
cvar_t r_aliastransadj=               {"r_aliastransadj", "100", 0, 0, 0, 0, 0};
cvar_t r_wateralpha=                       {"r_wateralpha", "1", 1, 0, 0, 0, 0};
cvar_t r_slimealpha=                       {"r_slimealpha", "1", 1, 0, 0, 0, 0};
cvar_t r_lavaalpha=                         {"r_lavaalpha", "1", 1, 0, 0, 0, 0};
cvar_t r_telealpha=                         {"r_telealpha", "1", 1, 0, 0, 0, 0};
cvar_t r_twopass=                             {"r_twopass", "1", 1, 0, 0, 0, 0};
cvar_t r_fogstyle=                           {"r_fogstyle", "3", 1, 0, 0, 0, 0};
cvar_t r_nofog=                                 {"r_nofog", "0", 1, 0, 0, 0, 0};
cvar_t r_alphastyle=                       {"r_alphastyle", "0", 1, 0, 0, 0, 0};
cvar_t r_entalpha=                           {"r_entalpha", "1", 1, 0, 0, 0, 0};
cvar_t r_labmixpal=                         {"r_labmixpal", "1", 1, 0, 0, 0, 0};
cvar_t r_rgblighting=                     {"r_rgblighting", "1", 1, 0, 0, 0, 0};
cvar_t r_fogbrightness=                 {"r_fogbrightness", "1", 1, 0, 0, 0, 0};
cvar_t r_fogfactor=                       {"r_fogfactor", "0.5", 1, 0, 0, 0, 0};
cvar_t r_fogscale=                           {"r_fogscale", "1", 1, 0, 0, 0, 0};
cvar_t r_fognoise=                           {"r_fognoise", "1", 1, 0, 0, 0, 0};
cvar_t r_litwater=                           {"r_litwater", "1", 1, 0, 0, 0, 0};
cvar_t vid_cwidth=                           {"vid_cwidth", "0", 1, 0, 0, 0, 0};
cvar_t vid_cheight=                         {"vid_cheight", "0", 1, 0, 0, 0, 0};
cvar_t vid_cwmode=                           {"vid_cwmode", "0", 1, 0, 0, 0, 0};

// johnfitz -- new cvars TODO actually implement these, they're currently placeholders
cvar_t  r_nolerp_list={"r_nolerp_list", "progs/flame.mdl,progs/flame2.mdl,progs/braztall.mdl,pro gs/brazshrt.mdl,progs/longtrch.mdl,progs/flame_pyre.mdl,progs/v_saw.mdl,progs/v_xfist.mdl,progs/h2 stuff/newfire.mdl", 0, 0, 0, 0};
cvar_t  r_noshadow_list={"r_noshadow_list", "progs/proj_balllava.mdl,progs/flame2.mdl,progs/flame.mdl,progs/bolt1.mdl,progs/bolt2.mdl,progs/bolt3.mdl,progs/laser.mdl", 0, 0, 0, 0};
// johnfitz
cvar_t r_fullbright_list={"r_fullbright_list", "progs/spike.mdl,progs/s_spike.mdl,progs/missile.mdl,progs/k_spike.mdl,progs/proj_balllava.mdl,progs/flame2.mdl,progs/flame.mdl,progs/bolt1.mdl,progs/bolt2.mdl,progs/bolt3.mdl,progs/laser.mdl", 0, 0, 0, 0};

