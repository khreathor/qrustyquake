/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// vid.h -- video driver defs

#ifndef __VID__
#define __VID__

#include "cvar.h"

#define VID_CBITS	6
#define VID_GRADES	(1 << VID_CBITS)

// a pixel can be one, two, or four bytes
typedef byte pixel_t;

typedef struct vrect_s
{
	int				x,y,width,height;
	struct vrect_s	*pnext;
} vrect_t;

typedef struct
{
	pixel_t			*buffer;		// invisible buffer
	pixel_t			*colormap;		// 256 * VID_GRADES size
	unsigned short	*colormap16;	// 256 * VID_GRADES size
	unsigned int		fullbright;		// index of first fullbright color
	unsigned int		rowbytes;	// may be > width if displayed in a window
	unsigned int		width;		
	unsigned int		height;
	float			aspect;		// width / height -- < 0 is taller than wide
	unsigned int				numpages;
	unsigned int				recalc_refdef;	// if true, recalc vid-based stuff
	pixel_t			*conbuffer;
	unsigned int				conrowbytes;
	unsigned		conwidth;
	unsigned		conheight;
	unsigned int				maxwarpwidth;
	unsigned int				maxwarpheight;
	pixel_t			*direct;		// direct drawing to framebuffer, if not
									//  NULL
} viddef_t;

#define NUM_OLDMODES 9
#define MAX_MODE_LIST 30
#define VID_ROW_SIZE 3
#define MAX_COLUMN_SIZE 5
#define MODE_AREA_HEIGHT (MAX_COLUMN_SIZE + 6)
#define MAX_MODEDESCS (MAX_COLUMN_SIZE * 3)
#define NO_MODE -1

extern unsigned int oldmodes[NUM_OLDMODES*2]; 
extern char modelist[NUM_OLDMODES][8]; // "320x240" etc. for menus
extern SDL_Window *window;
extern SDL_Surface *windowSurface;
extern SDL_Renderer *renderer;
extern SDL_Surface *argbbuffer;
extern SDL_Texture *texture;
extern SDL_Rect blitRect;
extern SDL_Rect destRect;
extern SDL_Surface *scaleBuffer;
extern SDL_Surface *screen;
extern unsigned int force_old_render;
extern unsigned int SDLWindowFlags;
extern unsigned int stretchpixels;
extern unsigned int uiscale;
extern unsigned int vimmode;
extern unsigned int VGA_width;
extern unsigned int VGA_height;
extern unsigned int VGA_rowbytes;
extern unsigned int VGA_bufferrowbytes;
extern unsigned char *VGA_pagebase;
extern int vid_line;
extern int lockcount;
extern int vid_modenum;
extern int vid_testingmode;
extern int vid_realmode;
extern int vid_default;
extern double vid_testendtime;
extern unsigned short d_8to16table[256];
extern unsigned char vid_curpal[256 * 3]; // save for mode changes
extern qboolean vid_initialized;
extern qboolean palette_changed;
extern viddef_t vid; // global video state
extern cvar_t _vid_default_mode_win;
extern cvar_t _windowed_mouse;
extern cvar_t scr_uiscale;
extern cvar_t scr_stretchpixels;
extern cvar_t newoptions;
extern cvar_t sensitivityyscale;
extern unsigned char vid_curpal[256*3];
void VID_SetMode (int modenum, int customw, int customh,
		int customwinmode, unsigned char *palette);
char *VID_GetModeDescription(int mode);
extern	unsigned short	d_8to16table[256];

void	VID_SetPalette (unsigned char *palette);
// called at startup and after any gamma correction

void	VID_Init (unsigned char *palette);
// Called at startup to set up translation tables, takes 256 8 bit RGB values
// the palette data will go away after the call, so it must be copied off if
// the video driver will need it again

void	VID_Shutdown (void);
// Called at shutdown

void	VID_CalcScreenDimensions();

void	VID_Update ();
// flushes the view buffer to the screen

void VID_SetMode(int modenum, int customw, int customh, int customwinmode,
                   unsigned char *palette);

void VID_HandlePause (qboolean pause);
// called only on Win32, when pause happens, so the mouse can be released
#endif
