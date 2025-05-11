// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

// quakedef.h -- primary header for client

#ifndef __QUAKEDEF__
#define __QUAKEDEF__

#ifndef __WIN32__
#include <SDL2/SDL.h>
#endif
#ifdef __WIN32__
#ifndef _WINCROSS
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#include <windows.h>
#include <time.h>
#include <direct.h>
#include <io.h>
#else
#include <unistd.h>
#include <sys/ipc.h>
#ifndef __HAIKU__
#include <sys/shm.h>
#endif
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/mman.h>
#endif
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <setjmp.h>
#include <float.h>

#include "defines.h"
#include "mathlib.h"

// the host system specifies the base of the directory tree, the
// command line parms passed to the program, and the amount of memory
// available for the program to use
typedef struct {
	char *basedir;
	char *userdir;
	char *cachedir; // for development over ISDN lines
	int argc;
	char **argv;
	void *membase;
	int memsize;
} quakeparms_t;

#include "typedefs.h"
#include "globals.h"
#include "common.h"
#include "sys.h"
#include "zone.h"
#include "wad.h"
#include "draw.h"
#include "cvar.h"
#include "screen.h"
#include "net.h"
#include "render.h"
#include "cmd.h"
#include "sbar.h"
#include "q_sound.h"
#include "progs.h"
#include "server.h"
#include "model.h"
#include "d_iface.h"
#include "input.h"
#include "world.h"
#include "keys.h"
#include "console.h"
#include "view.h"
#include "menu.h"
#include "crc.h"
#include "vgatext.h"
#include "cvarlist.h"
#include "client.h"

extern double host_time;
extern SDL_Window *window; // global for checking windowed state in options
extern qboolean noclip_anglehack;
extern quakeparms_t host_parms;
extern qboolean host_initialized; // true if into command execution
extern double host_frametime;
extern byte *host_basepal;
extern byte *host_colormap;
extern qboolean isDedicated;
extern int minimum_memory;
extern int host_framecount; // incremented every frame, never reset
extern double realtime; // not bounded in any way, changed at
			// start of every frame, never reset
extern qboolean msg_suppress_1; // suppresses resolution and cache size console
				// output an fullscreen DIB focus gain/loss
extern int current_skill; // skill level for currently loaded level (in case
			  // the user changes the cvar while the level is
			  // running, this reflects the level actually in use)

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
void Host_ShutdownServer(qboolean crash);
void Chase_Init();
void Chase_Update();
void Cvar_SetCallback(cvar_t *var, cvarcallback_t func);
#endif
