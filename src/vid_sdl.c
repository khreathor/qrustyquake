#include "quakedef.h"
#include "d_local.h"

#define NUM_OLDMODES 9
#define MAX_MODE_LIST 30
#define VID_ROW_SIZE 3
#define MAX_COLUMN_SIZE 5
#define MODE_AREA_HEIGHT (MAX_COLUMN_SIZE + 6)
#define MAX_MODEDESCS (MAX_COLUMN_SIZE * 3)
#define NO_MODE -1

unsigned int oldmodes[NUM_OLDMODES*2] = {
	320, 240,	640, 480,	800, 600,
	320, 200,	320, 240,	640, 350,
	640, 400,	640, 480,	800, 600
};
char modelist[NUM_OLDMODES][8]; // "320x240" etc. for menus
SDL_Window *window;
SDL_Surface *windowSurface;
SDL_Renderer *renderer;
SDL_Surface *argbbuffer;
SDL_Texture *texture;
SDL_Rect blitRect;
SDL_Rect destRect;
SDL_Surface *scaleBuffer;
SDL_Surface *screen;
unsigned int force_old_render;
unsigned int SDLWindowFlags;
unsigned int stretchpixels;
unsigned int uiscale;
unsigned int vimmode;
unsigned int VGA_width;
unsigned int VGA_height;
unsigned int VGA_rowbytes;
unsigned int VGA_bufferrowbytes;
unsigned char *VGA_pagebase;
int vid_line;
int lockcount;
int vid_modenum;
int vid_testingmode;
int vid_realmode;
int vid_default;
double vid_testendtime;
unsigned short d_8to16table[256];
unsigned char vid_curpal[256 * 3]; // save for mode changes
qboolean vid_initialized;
qboolean palette_changed;
qboolean mouse_avail;
float mouse_x;
float mouse_y;
int mouse_oldbuttonstate;
viddef_t vid; // global video state

cvar_t vid_mode = { "vid_mode", "0", 0, 0, 0, 0 };
cvar_t _vid_default_mode_win = { "_vid_default_mode_win", "3", 1, 0, 0, 0 };
cvar_t scr_uiscale = { "scr_uiscale", "1", 1, 0, 0, 0 };
cvar_t sensitivityyscale = { "sensitivityyscale", "1.0", 1, 0, 0, 0 };
cvar_t scr_stretchpixels = { "scr_stretchpixels", "0", 1, 0, 0, 0 };
cvar_t _windowed_mouse = { "_windowed_mouse", "0", 1, 0, 0, 0 };
cvar_t newoptions = { "newoptions", "1", 1, 0, 0, 0 };

void VID_CalcScreenDimensions();
void VID_AllocBuffers();

int VID_GetDefaultMode()
{
	// CyanBun96: _vid_default_mode_win gets read from config.cfg only after
	// video is initialized. To avoid creating a window and textures just to
	// destroy them right away and only then replace them with the valid ones
	// this function reads the default mode independently of cvar status
	FILE *file;
	char line[256];
	int vid_default_mode = -1;
	file = fopen("id1/config.cfg", "r");
	if (!file) {
		printf("Failed to open id1/config.cfg");
		return -2;
	}
	while (fgets(line, sizeof(line), file)) {
		char *key = "_vid_default_mode_win";
		char *found = strstr(line, key);
		if (found) {
			char *start = strchr(found, '"');
			if (start) {
				char *end = strchr(start + 1, '"');
				if (end) {
					*end = '\0';
					vid_default_mode = atoi(start + 1);
					break;
				}
			}
		}
	}
	fclose(file);
	if (vid_default_mode != -1)
		printf("_vid_default_mode_win: %d\n", vid_default_mode);
	else
		printf("_vid_default_mode_win not found in id1/config.cfg\n");
	return vid_default_mode;
}

int VID_DetermineMode()
{
	int win = !(SDLWindowFlags & (SDL_WINDOW_FULLSCREEN
				      | SDL_WINDOW_FULLSCREEN_DESKTOP));
	for (int i = 0; i < NUM_OLDMODES; ++i) {
		if (i > 2 ? !win : win
			&& vid.width == oldmodes[i*2]
			&& vid.height == oldmodes[i*2+1])
			return i;
	}
	return -1;
}

void VID_SetPalette(unsigned char *palette)
{
	SDL_Color colors[256];
	palette_changed = true;
	if (palette != vid_curpal)
		memcpy(vid_curpal, palette, sizeof(vid_curpal));
	for (int i = 0; i < 256; ++i) {
		colors[i].r = *palette++;
		colors[i].g = *palette++;
		colors[i].b = *palette++;
	}
	SDL_SetPaletteColors(screen->format->palette, colors, 0, 256);
}

void VID_Init(unsigned char *palette)
{
	int pnum;
	int flags;
	int winmode;
	int defmode;
	int realwidth;
	char caption[50];

	Cvar_RegisterVariable(&_windowed_mouse);
	Cvar_RegisterVariable(&_vid_default_mode_win);
	Cvar_RegisterVariable(&vid_mode);
	Cvar_RegisterVariable(&scr_uiscale);
	Cvar_RegisterVariable(&sensitivityyscale);
	Cvar_RegisterVariable(&scr_stretchpixels);
	Cvar_RegisterVariable(&newoptions);

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		Sys_Error("VID: Couldn't load SDL: %s", SDL_GetError());
	// Set up display mode (width and height)
	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	vid.width = 320;
	vid.height = 240;
	winmode = 0;
	defmode = VID_GetDefaultMode();
	if (defmode >= 0 && defmode < NUM_OLDMODES) {
		vid.width = oldmodes[defmode*2];
		vid.height = oldmodes[defmode*2+1];
		winmode = defmode < 3;
	}
	if ((pnum = COM_CheckParm("-winsize"))) {
		if (pnum >= com_argc - 2)
			Sys_Error("VID: -winsize <width> <height>\n");
		vid.width = Q_atoi(com_argv[pnum + 1]);
		vid.height = Q_atoi(com_argv[pnum + 2]);
		if (!vid.width || !vid.height)
			Sys_Error("VID: Bad window width/height\n");
	}
	if ((pnum = COM_CheckParm("-width"))) {
		if (pnum >= com_argc - 1)
			Sys_Error("VID: -width <width>\n");
		vid.width = Q_atoi(com_argv[pnum + 1]);
		if (!vid.width)
			Sys_Error("VID: Bad window width\n");
	}
	if ((pnum = COM_CheckParm("-height"))) {
		if (pnum >= com_argc - 1)
			Sys_Error("VID: -height <height>\n");
		vid.height = Q_atoi(com_argv[pnum + 1]);
		if (!vid.height)
			Sys_Error("VID: Bad window height\n");
	}
	// In windowed mode, the window can be resized at runtime
	flags = SDL_WINDOW_RESIZABLE;
	// Set the highdpi flag - this makes a big difference on Macs with
	// retina displays, especially when using small window sizes.
	flags |= SDL_WINDOW_ALLOW_HIGHDPI;
	// CyanBun96: what most modern games call "Fullscreen borderless
	// can be achieved by passing -fullscreen_desktop and -borderless
	// -fullscreen will try to change the display resolution
	if (COM_CheckParm("-fullscreen"))
		flags |= SDL_WINDOW_FULLSCREEN;
	else if (COM_CheckParm("-fullscreen_desktop"))
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	else if (COM_CheckParm("-window"))
		flags &= ~SDL_WINDOW_FULLSCREEN;
	else if (winmode == 0)
		flags |= SDL_WINDOW_FULLSCREEN;
	else if (winmode == 1)
		flags &= ~SDL_WINDOW_FULLSCREEN;
	if (COM_CheckParm("-borderless"))
		flags |= SDL_WINDOW_BORDERLESS;
	if (COM_CheckParm("-forceoldrender"))
		force_old_render = 1;
	else
		force_old_render = 0;
	if (COM_CheckParm("-vimmode"))
		vimmode = 1;
	if (vid.width > 1280 || vid.height > 1024) 
		Sys_Printf("vanilla maximum resolution is 1280x1024\n");
	if (COM_CheckParm("-stretchpixels")
	    || vid.height == 200 || vid.height == 400)
		stretchpixels = 1;
	else
		stretchpixels = 0;
	Cvar_SetValue("scr_stretchpixels", stretchpixels);
	realwidth = vid.width - (vid.width - vid.width / 1.2) * stretchpixels;
	window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED,
						SDL_WINDOWPOS_CENTERED,
						realwidth, vid.height, flags);
	screen = SDL_CreateRGBSurfaceWithFormat(0,
						vid.width,
						vid.height,
						8, SDL_PIXELFORMAT_INDEX8);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!force_old_render) {
		argbbuffer = SDL_CreateRGBSurfaceWithFormatFrom(NULL,
						vid.width,
						vid.height,
						0,
						0,
						SDL_PIXELFORMAT_ARGB8888);
		texture = SDL_CreateTexture(renderer,
						SDL_PIXELFORMAT_ARGB8888,
						SDL_TEXTUREACCESS_STREAMING,
						vid.width, vid.height);
	} else {
		scaleBuffer = SDL_CreateRGBSurfaceWithFormat(0,
						vid.width,
						vid.height,
						8,
						SDL_GetWindowPixelFormat
						(window));
	}
	windowSurface = SDL_GetWindowSurface(window);
	VID_CalcScreenDimensions();
	sprintf(caption, "QrustyQuake - Version %4.2f", VERSION);
	SDL_SetWindowTitle(window, (const char *)&caption);
	SDLWindowFlags = SDL_GetWindowFlags(window);
	VID_SetPalette(palette);
	VGA_width = vid.conwidth = vid.width;
	VGA_height = vid.conheight = vid.height;
	vid.aspect = ((float)vid.height / (float)vid.width) * (320.0 / 240.0);
	vid.numpages = 1;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong(*((int *)vid.colormap + 2048));
	VGA_pagebase = vid.buffer = screen->pixels;
	VGA_rowbytes = vid.rowbytes = screen->pitch;
	vid.conbuffer = vid.buffer;
	vid.conrowbytes = vid.rowbytes;
	vid.direct = (pixel_t *) screen->pixels;
	// allocate z buffer and surface cache
	VID_AllocBuffers();
	// initialize the mouse
	SDL_ShowCursor(0);
	vid_initialized = true;
	vid_modenum = VID_DetermineMode();
	if (vid_modenum < 0)
		Con_Printf("WARNING: non-standard video mode\n");
	else
		Con_Printf("Detected video mode %d\n", vid_modenum);
}

void VID_Shutdown(void)
{
	if (vid_initialized) {
		if (screen != NULL && lockcount > 0)
			SDL_UnlockSurface(screen);
		if (!force_old_render)
			SDL_UnlockTexture(texture);
		vid_initialized = 0;
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
	}
}

void VID_CalcScreenDimensions()
{
	uiscale = (vid.width / 320);
	if (uiscale * 200 > vid.height)
		uiscale = 1;	// For weird resolutions like 800x200
	Cvar_SetValue("scr_uiscale", uiscale);
	// Scaling code, courtesy of ChatGPT
	// Original, pre-scale screen size
	blitRect.x = 0;
	blitRect.y = 0;
	blitRect.w = vid.width;
	blitRect.h = vid.height;
	// Get window dimensions
	int winW = windowSurface->w;
	int winH = windowSurface->h;
	// Get scaleBuffer dimensions
	int bufW = vid.width;
	int bufH = vid.height;
	float bufAspect = (float)bufW / bufH;
	if (stretchpixels)
		bufAspect /= 1.2;
	// Calculate scaled dimensions
	int destW, destH;
	if ((float)winW / winH > bufAspect) {
		// Window is wider than buffer, black bars on sides
		destH = winH;
		destW = (int)(winH * bufAspect);
	} else {
		// Window is taller than buffer, black bars on top/bottom
		destW = winW;
		destH = (int)(winW / bufAspect);
	}
	// Center the destination rectangle
	destRect.x = (winW - destW) / 2;
	destRect.y = (winH - destH) / 2;
	destRect.w = destW;
	destRect.h = destH;
}

void VID_Update()
{
	// Machines without a proper GPU will try to simulate one with software,
	// adding a lot of overhead. In my tests, software rendering accomplished
	// the same result with almost a 200% performance increase.
	if (force_old_render) {	// pure software rendering
		SDL_UpperBlit(screen, NULL, scaleBuffer, NULL);
		SDL_UpperBlitScaled(scaleBuffer, &blitRect, windowSurface,
				    &destRect);
		SDL_UpdateWindowSurface(window);
	} else {		// hardware-accelerated rendering
		SDL_LockTexture(texture, &blitRect, &argbbuffer->pixels,
				&argbbuffer->pitch);
		SDL_LowerBlit(screen, &blitRect, argbbuffer, &blitRect);
		SDL_UnlockTexture(texture);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, &destRect);
		SDL_RenderPresent(renderer);
	}
	if (uiscale != scr_uiscale.value
	    && vid.width / 320 >= scr_uiscale.value && scr_uiscale.value > 0) {
		uiscale = scr_uiscale.value;
		vid.recalc_refdef = 1;
	}
	if (stretchpixels != scr_stretchpixels.value) {
		stretchpixels = scr_stretchpixels.value;
		VID_CalcScreenDimensions();
	}
	if (vid_testingmode) {
		if (realtime >= vid_testendtime) {
			VID_SetMode(vid_realmode, 0, 0, 0, vid_curpal);
			vid_testingmode = 0;
		}
	} else if ((int)vid_mode.value != vid_realmode) {
		VID_SetMode((int)vid_mode.value, 0, 0, 0, vid_curpal);
		Cvar_SetValue("vid_mode", (float)vid_modenum);
		vid_realmode = vid_modenum;
	}
}

void D_BeginDirectRect(int x, int y, byte *pbitmap, int width, int height)
{
	Uint8 *offset;
	if (!screen)
		return;
	if (x < 0)
		x = screen->w + x - 1;
	offset = (Uint8 *) screen->pixels + y * screen->pitch + x;
	while (height--) {
		memcpy(offset, pbitmap, width);
		offset += screen->pitch;
		pbitmap += width;
	}
}

// TODO: what the beep is input doing in the fucking video source anyway?
void Sys_SendKeyEvents(void)
{
	SDL_Event event;
	int sym, state;
	int modstate;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			sym = event.key.keysym.sym;
			state = event.key.state;
			modstate = SDL_GetModState();
			switch (sym) {
			case SDLK_DELETE:
				sym = K_DEL;
				break;
			case SDLK_BACKSPACE:
				sym = K_BACKSPACE;
				break;
			case SDLK_F1:
				sym = K_F1;
				break;
			case SDLK_F2:
				sym = K_F2;
				break;
			case SDLK_F3:
				sym = K_F3;
				break;
			case SDLK_F4:
				sym = K_F4;
				break;
			case SDLK_F5:
				sym = K_F5;
				break;
			case SDLK_F6:
				sym = K_F6;
				break;
			case SDLK_F7:
				sym = K_F7;
				break;
			case SDLK_F8:
				sym = K_F8;
				break;
			case SDLK_F9:
				sym = K_F9;
				break;
			case SDLK_F10:
				sym = K_F10;
				break;
			case SDLK_F11:
				sym = K_F11;
				break;
			case SDLK_F12:
				sym = K_F12;
				break;
				//case SDLK_BREAK:
			case SDLK_PAUSE:
				sym = K_PAUSE;
				break;

				// CyanBun96: vim-like keybinds that work in menus
			case SDLK_h:
				sym = vimmode ? K_LEFTARROW : 'h';
				break;
			case SDLK_j:
				sym = vimmode ? K_DOWNARROW : 'j';
				break;
			case SDLK_k:
				sym = vimmode ? K_UPARROW : 'k';
				break;
			case SDLK_l:
				sym = vimmode ? K_RIGHTARROW : 'l';
				break;

			case SDLK_UP:
				sym = K_UPARROW;
				break;
			case SDLK_DOWN:
				sym = K_DOWNARROW;
				break;
			case SDLK_RIGHT:
				sym = K_RIGHTARROW;
				break;
			case SDLK_LEFT:
				sym = K_LEFTARROW;
				break;
			case SDLK_INSERT:
				sym = K_INS;
				break;
			case SDLK_HOME:
				sym = K_HOME;
				break;
			case SDLK_END:
				sym = K_END;
				break;
			case SDLK_PAGEUP:
				sym = K_PGUP;
				break;
			case SDLK_PAGEDOWN:
				sym = K_PGDN;
				break;
			case SDLK_RSHIFT:
			case SDLK_LSHIFT:
				sym = K_SHIFT;
				break;
			case SDLK_RCTRL:
			case SDLK_LCTRL:
				sym = K_CTRL;
				break;
			case SDLK_RALT:
			case SDLK_LALT:
				sym = K_ALT;
				break;
			case SDLK_KP_0:
				if (modstate & KMOD_NUM)
					sym = K_INS;
				else
					sym = SDLK_0;
				break;
			case SDLK_KP_1:
				if (modstate & KMOD_NUM)
					sym = K_END;
				else
					sym = SDLK_1;
				break;
			case SDLK_KP_2:
				if (modstate & KMOD_NUM)
					sym = K_DOWNARROW;
				else
					sym = SDLK_2;
				break;
			case SDLK_KP_3:
				if (modstate & KMOD_NUM)
					sym = K_PGDN;
				else
					sym = SDLK_3;
				break;
			case SDLK_KP_4:
				if (modstate & KMOD_NUM)
					sym = K_LEFTARROW;
				else
					sym = SDLK_4;
				break;
			case SDLK_KP_5:
				sym = SDLK_5;
				break;
			case SDLK_KP_6:
				if (modstate & KMOD_NUM)
					sym = K_RIGHTARROW;
				else
					sym = SDLK_6;
				break;
			case SDLK_KP_7:
				if (modstate & KMOD_NUM)
					sym = K_HOME;
				else
					sym = SDLK_7;
				break;
			case SDLK_KP_8:
				if (modstate & KMOD_NUM)
					sym = K_UPARROW;
				else
					sym = SDLK_8;
				break;
			case SDLK_KP_9:
				if (modstate & KMOD_NUM)
					sym = K_PGUP;
				else
					sym = SDLK_9;
				break;
			case SDLK_KP_PERIOD:
				if (modstate & KMOD_NUM)
					sym = K_DEL;
				else
					sym = SDLK_PERIOD;
				break;
			case SDLK_KP_DIVIDE:
				sym = SDLK_SLASH;
				break;
			case SDLK_KP_MULTIPLY:
				sym = SDLK_ASTERISK;
				break;
			case SDLK_KP_MINUS:
				sym = SDLK_MINUS;
				break;
			case SDLK_KP_PLUS:
				sym = SDLK_PLUS;
				break;
			case SDLK_KP_ENTER:
				sym = SDLK_RETURN;
				break;
			case SDLK_KP_EQUALS:
				sym = SDLK_EQUALS;
				break;
			}
			// If we're not directly handled and still above 255
			// just force it to 0
			if (sym > 255)
				sym = 0;
			Key_Event(sym, state);
			break;
		// WinQuake behavior: Use Mouse OFF disables mouse input entirely
		// ON grabs the mouse, kinda like SetRelativeMouseMode(SDL_TRUE)
		// Fullscreen grabs the mouse unconditionally
		case SDL_MOUSEMOTION:
			if ((event.motion.x != ((int)vid.width / 2)) ||
			    (event.motion.y != ((int)vid.height / 2))) {
				mouse_x = event.motion.xrel * 10;
				mouse_y = event.motion.yrel * 10;
			}
			break;
		case SDL_WINDOWEVENT:
			switch (event.window.event) {
			case SDL_WINDOWEVENT_RESIZED:
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				SDLWindowFlags = SDL_GetWindowFlags(window);
				windowSurface = SDL_GetWindowSurface(window);
				VID_CalcScreenDimensions();
				break;
			}
			break;
		case SDL_QUIT:
			CL_Disconnect();
			Host_ShutdownServer(false);
			Sys_Quit();
			break;
		default:
			break;
		}
	}
}

void IN_Init(void)
{
	if (COM_CheckParm("-nomouse"))
		return;
	mouse_x = mouse_y = 0.0;
	mouse_avail = 1;
}

void IN_Shutdown(void)
{
	mouse_avail = 0;
}

void IN_Commands(void)
{
	int i;
	int mouse_buttonstate;

	if (!mouse_avail)
		return;

	i = SDL_GetMouseState(NULL, NULL);
	/* Quake swaps the second and third buttons */
	mouse_buttonstate = (i & ~0x06) | ((i & 0x02) << 1) | ((i & 0x04) >> 1);
	for (i = 0; i < 3; i++) {
		if ((mouse_buttonstate & (1 << i))
		    && !(mouse_oldbuttonstate & (1 << i)))
			Key_Event(K_MOUSE1 + i, true);

		if (!(mouse_buttonstate & (1 << i))
		    && (mouse_oldbuttonstate & (1 << i)))
			Key_Event(K_MOUSE1 + i, false);
	}
	mouse_oldbuttonstate = mouse_buttonstate;
}

void IN_Move(usercmd_t *cmd)
{
	if (!mouse_avail)
		return;
	if (!(SDLWindowFlags & (SDL_WINDOW_FULLSCREEN
				| SDL_WINDOW_FULLSCREEN_DESKTOP))
	    && !_windowed_mouse.value) {
		SDL_SetRelativeMouseMode(SDL_FALSE);
		return;
	}
	SDL_SetRelativeMouseMode(SDL_TRUE);

	mouse_x *= sensitivity.value;
	mouse_y *= sensitivity.value * sensitivityyscale.value;

	if ((in_strafe.state & 1) || (lookstrafe.value && (in_mlook.state & 1)))
		cmd->sidemove += m_side.value * mouse_x;
	else
		cl.viewangles[YAW] -= m_yaw.value * mouse_x;
	if (in_mlook.state & 1)
		V_StopPitchDrift();

	if ((in_mlook.state & 1) && !(in_strafe.state & 1)) {
		cl.viewangles[PITCH] += m_pitch.value * mouse_y;
		if (cl.viewangles[PITCH] > 80)
			cl.viewangles[PITCH] = 80;
		if (cl.viewangles[PITCH] < -70)
			cl.viewangles[PITCH] = -70;
	} else {
		if ((in_strafe.state & 1) && noclip_anglehack)
			cmd->upmove -= m_forward.value * mouse_y;
		else
			cmd->forwardmove -= m_forward.value * mouse_y;
	}
	mouse_x = mouse_y = 0.0;
}

char *VID_GetModeDescription(int mode)
{
	static char pinfo[40];
	if ((mode < 0) || (mode >= NUM_OLDMODES))
		sprintf(pinfo, "Custom fullscreen");
	else if (mode >= 3) 
		sprintf(pinfo, "%s fullscreen", modelist[mode]);
	else
		sprintf(pinfo, "%s windowed", modelist[mode]);
	return pinfo;
}

void VID_AllocBuffers()
{
	// allocate z buffer and surface cache
	int chunk = vid.width * vid.height * sizeof(*d_pzbuffer);
	int cachesize = D_SurfaceCacheForRes(vid.width, vid.height);
	chunk += cachesize;
	d_pzbuffer = Hunk_HighAllocName(chunk, "video");
	if (d_pzbuffer == NULL)
		Sys_Error("Not enough memory for video mode\n");
	unsigned char *cache = (byte *) d_pzbuffer
	    + vid.width * vid.height * sizeof(*d_pzbuffer);
	D_InitCaches(cache, cachesize);
}

void VID_SetMode(int modenum, int customw, int customh, int customwinmode,
		   unsigned char *palette)
{
	if (modenum == vid_modenum)
		return;
	if (customw && customh) {
		vid.width = customw;
		vid.height = customh;
		vid_modenum = -1;
	} else {
		vid.width = oldmodes[modenum*2];
		vid.height = oldmodes[modenum*2+1];
		stretchpixels = modenum == 3 || modenum == 6;
		Cvar_SetValue("scr_stretchpixels", stretchpixels);
		vid_modenum = modenum;
	}
	// CyanBun96: why do surfaces get freed but textures get destroyed :(
	SDL_FreeSurface(screen);
	screen = SDL_CreateRGBSurfaceWithFormat(0, vid.width, vid.height, 8,
						SDL_PIXELFORMAT_INDEX8);
	if (!force_old_render) {
		SDL_FreeSurface(argbbuffer);
		argbbuffer = SDL_CreateRGBSurfaceWithFormatFrom(NULL,
						vid.width, vid.height, 0, 0,
						SDL_PIXELFORMAT_ARGB8888);
		SDL_DestroyTexture(texture);
		texture = SDL_CreateTexture(renderer,
						SDL_PIXELFORMAT_ARGB8888,
						SDL_TEXTUREACCESS_STREAMING,
						vid.width, vid.height);
	} else {
		SDL_FreeSurface(scaleBuffer);
		scaleBuffer = SDL_CreateRGBSurfaceWithFormat(0,
						vid.width, vid.height, 8,
						SDL_GetWindowPixelFormat
						(window));
	}
	VGA_width = vid.conwidth = vid.width;
	VGA_height = vid.conheight = vid.height;
	vid.aspect = ((float)vid.height / (float)vid.width) * (320.0 / 240.0);
	VGA_pagebase = vid.buffer = screen->pixels;
	VGA_rowbytes = vid.rowbytes = screen->pitch;
	vid.conbuffer = vid.buffer;
	vid.conrowbytes = vid.rowbytes;
	vid.direct = (pixel_t *) screen->pixels;
	VID_AllocBuffers();
	vid.recalc_refdef = 1;
	VID_CalcScreenDimensions();
	VID_SetPalette(palette);
	if (!customw || !customh) {
		if (modenum <= 2) {
			SDL_SetWindowFullscreen(window, 0);
			SDL_SetWindowSize(window, vid.width, vid.height);
			SDL_SetWindowPosition(window,
					      SDL_WINDOWPOS_CENTERED,
					      SDL_WINDOWPOS_CENTERED);
		} else {
			SDL_SetWindowFullscreen(window,
						SDL_WINDOW_FULLSCREEN_DESKTOP);
		}
	} else {
		if (customwinmode == 0) {
			SDL_SetWindowFullscreen(window, 0);
			SDL_SetWindowSize(window, vid.width, vid.height);
			SDL_SetWindowPosition(window,
					      SDL_WINDOWPOS_CENTERED,
					      SDL_WINDOWPOS_CENTERED);
		} else if (customwinmode == 1) {
			SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
		} else {
			SDL_SetWindowFullscreen(window,
						SDL_WINDOW_FULLSCREEN_DESKTOP);
		}
	}
	Cvar_SetValue("vid_mode", (float)vid_modenum);
}
