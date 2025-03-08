// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

// the status bar is only redrawn if something has changed, but if anything
// does, the entire thing will be redrawn for the next vid.numpages frames.
extern int sb_lines; // scan lines to draw
extern cvar_t scr_sidescore;
void Sbar_Init();
void Sbar_Changed(); // call whenever any of the client stats represented on the sbar changes
void Sbar_Draw(); // called every frame by screen
void Sbar_IntermissionOverlay(); // called each frame after the level has been completed
void Sbar_FinaleOverlay();
void Sbar_CalcPos(); // call on each sbar size change
