// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

void SCR_Init ();
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

typedef enum hudstyle_t
{
	HUD_CLASSIC,
	HUD_MODERN_CENTERAMMO, // Modern 1
	HUD_MODERN_SIDEAMMO, // Modern 2
	HUD_QUAKEWORLD,
	HUD_COUNT,
} hudstyle_t;

extern hudstyle_t hudstyle;
