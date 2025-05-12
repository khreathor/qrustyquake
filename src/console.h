// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

extern int con_totallines;
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
