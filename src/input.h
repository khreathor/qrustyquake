// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

void IN_Init ();
void IN_Shutdown ();
void IN_Commands ();
// oportunity for devices to stick commands on the script buffer
void IN_Move (usercmd_t *cmd);
// add additional movement on top of the keyboard move cmd
void IN_ClearStates ();
// restores all button and position states to defaults
void Sys_SendKeyEvents ();
