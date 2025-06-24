#include "quakedef.h"

static SDL_Renderer *renderer;
static SDL_Surface *argbbuffer;
static SDL_Texture *texture;
static SDL_Rect blitRect;
static SDL_Rect destRect;
static SDL_Rect scRect;
static SDL_Surface *scaleBuffer;
static u32 force_old_render;
static s32 VID_highhunkmark;

void VID_CalcScreenDimensions(cvar_t *cvar);
void VID_AllocBuffers();

s32 VID_GetConfigCvar(const s8 *cvname)
{
	// CyanBun96: _vid_default_mode_win gets read from config.cfg only after
	// video is initialized. To avoid creating a window and textures just to
	// destroy them right away and only then replace them with valid ones,
	// this function reads the default mode independently of the cvar status
	s8 line[256];
	s32 ret = -1;
	FILE *file = fopen("id1/config.cfg", "r");
	if(!file){
		printf("VID_GetConfigCvar: Failed to open id1/config.cfg\n");
		return -2;
	}
	while(fgets(line, sizeof(line), file)){
		s8 *found = strstr(line, cvname);
		if(found){
			s8 *start = strchr(found, '"');
			if(start){
				s8 *end = strchr(start + 1, '"');
				if(end){
					*end = '\0';
					ret = atoi(start + 1);
					break;
				}
			}
		}
	}
	fclose(file);
	if(ret != -1)
		printf("%s: %d\n", cvname, ret);
	else
		printf("%s not found in id1/config.cfg\n", cvname);
	return ret;
}

s32 VID_DetermineMode()
{
	s32 win = !(SDLWindowFlags & (SDL_WINDOW_FULLSCREEN
				      | SDL_WINDOW_FULLSCREEN_DESKTOP));
	for(s32 i = 0; i < NUM_OLDMODES; ++i){
		if(i > 2 ? !win : win && vid.width == oldmodes[i * 2]
		    && vid.height == oldmodes[i * 2 + 1])
			return i;
	}
	return -1;
}

void VID_SetPalette(u8 *palette, SDL_Surface *dest)
{
	SDL_Color colors[256];
	if(palette != vid_curpal)
		memcpy(vid_curpal, palette, sizeof(vid_curpal));
	for(s32 i = 0; i < 256; ++i){
		colors[i].r = *palette++;
		colors[i].g = *palette++;
		colors[i].b = *palette++;
	}
	SDL_SetPaletteColors(dest->format->palette, colors, 0, 256);
}

void VID_Init(u8 */*palette*/)
{
	s32 pnum;
	s32 flags;
	s32 winmode;
	s32 defmode = -1;
	s8 caption[50];
	s32 winW, winH;

	Cvar_RegisterVariable(&_windowed_mouse);
	Cvar_RegisterVariable(&_vid_default_mode_win);
	Cvar_RegisterVariable(&vid_mode);
	Cvar_RegisterVariable(&scr_uiscale);
	Cvar_RegisterVariable(&sensitivityyscale);
	Cvar_RegisterVariable(&newoptions);
	Cvar_RegisterVariable(&aspectr);
	Cvar_RegisterVariable(&realwidth);
	Cvar_RegisterVariable(&realheight);
	void (*vid_callback)(cvar_t *) =
		(void (*)(cvar_t *))VID_CalcScreenDimensions;
	Cvar_SetCallback(&aspectr, vid_callback);
	Cvar_SetCallback(&realwidth, vid_callback);
	Cvar_SetCallback(&realheight, vid_callback);
	// Set up display mode (width and height)
	vid.width = 320;
	vid.height = 240;
	winmode = 0;
	if(!(COM_CheckParm("-width") || COM_CheckParm("-height")
	   || COM_CheckParm("-window") || COM_CheckParm("-fullscreen")
	   || COM_CheckParm("-winsize"))){
		defmode = VID_GetConfigCvar("_vid_default_mode_win");
		if(defmode >= 0 && defmode < NUM_OLDMODES){
			vid.width = oldmodes[defmode * 2];
			vid.height = oldmodes[defmode * 2 + 1];
			winmode = defmode < 3;
		}
	}
	s32 confwidth = VID_GetConfigCvar("vid_cwidth");
	s32 confheight = VID_GetConfigCvar("vid_cheight");
	s32 confwmode = VID_GetConfigCvar("vid_cwmode");
	if(confwidth >= 320 && confheight >= 200 &&
		confwidth <= MAXWIDTH && confheight <= MAXHEIGHT){
		vid.width = confwidth;
		vid.height = confheight;
		Sys_Printf("Using vid_cwidth x vid_cheight to set mode\n");
	}
	else if(defmode != -1)
		Sys_Printf("Using _vid_default_mode_win to set mode\n");
	if((pnum = COM_CheckParm("-winsize"))){
		if(pnum >= com_argc - 2)
			Sys_Error("VID: -winsize <width> <height>\n");
		vid.width = Q_atoi(com_argv[pnum + 1]);
		vid.height = Q_atoi(com_argv[pnum + 2]);
		if(!vid.width || !vid.height)
			Sys_Error("VID: Bad window width/height\n");
	}
	if((pnum = COM_CheckParm("-width"))){
		if(pnum >= com_argc - 1)
			Sys_Error("VID: -width <width>\n");
		vid.width = Q_atoi(com_argv[pnum + 1]);
		if(!vid.width)
			Sys_Error("VID: Bad window width\n");
	}
	if((pnum = COM_CheckParm("-height"))){
		if(pnum >= com_argc - 1)
			Sys_Error("VID: -height <height>\n");
		vid.height = Q_atoi(com_argv[pnum + 1]);
		if(!vid.height)
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
	if(COM_CheckParm("-fullscreen") || confwmode == 1)
		flags |= SDL_WINDOW_FULLSCREEN;
	else if(COM_CheckParm("-fullscreen_desktop") || confwmode == 2)
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	else if(COM_CheckParm("-window") || confwmode == 0)
		flags &= ~SDL_WINDOW_FULLSCREEN;
	else if(winmode == 0)
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	else if(winmode == 1)
		flags &= ~SDL_WINDOW_FULLSCREEN;
	if(COM_CheckParm("-borderless"))
		flags |= SDL_WINDOW_BORDERLESS;
	force_old_render = COM_CheckParm("-forceoldrender");
#ifdef _WIN32
	force_old_render = 1; // layers don't work on windeez, FIXME
#endif
	vimmode = COM_CheckParm("-vimmode");
	if(vid.width > 1280 || vid.height > 1024)
		Sys_Printf("vanilla maximum resolution is 1280x1024\n");
	aspectr.value = 1.333333;
	realwidth.value = vid.width;
	realheight.value = (s32)(vid.width / aspectr.value + 0.5);
	window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_UNDEFINED,
				  SDL_WINDOWPOS_UNDEFINED,
				  realwidth.value, realheight.value, flags);
	screen = SDL_CreateRGBSurfaceWithFormat(0, vid.width, vid.height,
		8, SDL_PIXELFORMAT_INDEX8);
	screentop = SDL_CreateRGBSurfaceWithFormat(0, vid.width, vid.height,
		8, SDL_PIXELFORMAT_INDEX8);
	screenui = SDL_CreateRGBSurfaceWithFormat(0, vid.width, vid.height,
		8, SDL_PIXELFORMAT_INDEX8);
	scrbuffs[0] = screen; scrbuffs[1] = screentop; scrbuffs[2] = screenui;
	VID_SetPalette(host_basepal, screentop);
	SDL_SetColorKey(screentop, 1, 255);
	VID_SetPalette(host_basepal, screenui);
	SDL_SetColorKey(screenui, 1, 255);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	renderer = SDL_CreateRenderer(window, -1, 0);
	if(!force_old_render){
		SDL_GetWindowSizeInPixels(window, &winW, &winH);
		Sys_Printf("Size in Pixels: %d x %d\n", winW, winH);
		argbbuffer = SDL_CreateRGBSurfaceWithFormatFrom(NULL, winW,
			winH, 0, 0, SDL_PIXELFORMAT_ARGB8888);
		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING, vid.width, vid.height);
	} else {
		scaleBuffer = SDL_CreateRGBSurfaceWithFormat(0, vid.width,
			vid.height, 8, SDL_GetWindowPixelFormat (window));
	}
	sprintf(caption, "QrustyQuake - Version %4.2f", VERSION);
	SDL_SetWindowTitle(window, (const s8 *)&caption);
	SDLWindowFlags = SDL_GetWindowFlags(window);
	vid.aspect = ((f32)vid.height / (f32)vid.width) * (320.0 / 240.0);
	vid.numpages = 1;
	vid.colormap = host_colormap;
	vid.buffer = screen->pixels;
	VID_AllocBuffers(); // allocate z buffer, surface cache and the fbuffer
	SDL_ShowCursor(0); // initialize the mouse
	if(defmode >= 0)
		vid_modenum = defmode;
	else
		vid_modenum = VID_DetermineMode();
	if(vid_modenum < 0)
		Con_Printf("WARNING: non-standard video mode\n");
	else
		Con_Printf("Detected video mode %d\n", vid_modenum);
	realwidth.value = 0;
	VID_CalcScreenDimensions(0);
}

void VID_Shutdown()
{
	if(screen != NULL)
		SDL_UnlockSurface(screen);
	if(!force_old_render)
		SDL_UnlockTexture(texture);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void VID_CalcScreenDimensions(cvar_t */*cvar*/)
{
	uiscale = (vid.width / 320);
	if(uiscale * 200 > vid.height)
		uiscale = 1; // For weird resolutions like 800x200
	Cvar_SetValue("scr_uiscale", uiscale);
	// Scaling code, courtesy of ChatGPT
	// Original, pre-scale screen size
	blitRect.x = 0;
	blitRect.y = 0;
	blitRect.w = vid.width;
	blitRect.h = vid.height;
	// Get window dimensions
	s32 winW, winH;
	if(realwidth.value == 0 || realheight.value == 0){
		SDL_GetWindowSizeInPixels(window, &winW, &winH);
	}
	else {
		winW = realwidth.value;
		winH = realheight.value;
	}
	// Get scaleBuffer dimensions
	s32 bufW = vid.width;
	s32 bufH = vid.height;
	f32 bufAspect;
	if(aspectr.value == 0){
		bufAspect = (f32)bufW / bufH;
	}
	else
		bufAspect = aspectr.value;
	Cvar_SetValue("aspectr", bufAspect);
	// Calculate scaled dimensions
	s32 destW, destH;
	if((f32)winW / winH > bufAspect){
		// Window is wider than buffer, black bars on sides
		destH = winH;
		destW = (s32)(winH * bufAspect + 0.5);
	} else {
		// Window is taller than buffer, black bars on top/bottom
		destW = winW;
		destH = (s32)(winW / bufAspect + 0.5);
	}
	// Center the destination rectangle
	destRect.x = (winW - destW) / 2;
	destRect.y = (winH - destH) / 2;
	destRect.w = destW;
	destRect.h = destH;
	Sbar_CalcPos();
}

void VID_Update()
{
	scRect.w = vid.width * (scr_uixscale.value>0 ? scr_uixscale.value : 1);
	scRect.h = vid.height * (scr_uiyscale.value>0 ? scr_uiyscale.value : 1);
	scRect.x = (vid.width - scRect.w) / 2;
	scRect.y = (vid.height - scRect.h) / 2;
	// Machines without a proper GPU will try to simulate one with software,
	// adding a lot of overhead. In my tests software rendering accomplished
	// the same result with almost a 200% performance increase.
	if(force_old_render){ // pure software rendering
		SDL_UpperBlit(screen, NULL, scaleBuffer, NULL);
		SDL_UpperBlitScaled(screenui, &blitRect, scaleBuffer, &scRect);
		SDL_UpperBlit(screentop, NULL, scaleBuffer, NULL);
		SDL_UpperBlitScaled(scaleBuffer, &blitRect, windowSurface, &destRect);
		SDL_UpdateWindowSurface(window);
	} else { // hardware-accelerated rendering
		SDL_LockTexture(texture, &blitRect, &argbbuffer->pixels,
				&argbbuffer->pitch);
		SDL_LowerBlit(screen, &blitRect, argbbuffer, &blitRect);
		// Temporary fix - performance gets a hit because of this operation
		converted = SDL_ConvertSurfaceFormat(screenui, SDL_PIXELFORMAT_RGB888, 0);
		SDL_LowerBlitScaled(converted, &blitRect, argbbuffer, &scRect);
		SDL_LowerBlit(screentop, &blitRect, argbbuffer, &blitRect);
		SDL_UnlockTexture(texture);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, &destRect);
		SDL_RenderPresent(renderer);
	}
	if(uiscale != scr_uiscale.value // TODO make this a callback function
	    && vid.width / 320 >= scr_uiscale.value && scr_uiscale.value > 0){
		uiscale = scr_uiscale.value;
		vid.recalc_refdef = 1;
	}
	if(vid_testingmode){
		if(realtime >= vid_testendtime){
			VID_SetMode(vid_realmode, 0, 0, 0, vid_curpal);
			vid_testingmode = 0;
		}
	} else if((s32)vid_mode.value != vid_realmode && vid_realmode != -1){
		VID_SetMode((s32)vid_mode.value, 0, 0, 0, vid_curpal);
		Cvar_SetValue("vid_mode", (f32)vid_modenum);
		vid_realmode = vid_modenum;
	}
	memset(screentop->pixels, 255, vid.width*vid.height);
	memset(screenui->pixels, 255, vid.width*vid.height);
}

s8 *VID_GetModeDescription(s32 mode)
{
	static s8 pinfo[40];
	if((mode < 0) || (mode >= NUM_OLDMODES))
		sprintf(pinfo, "Custom fullscreen");
	else if(mode >= 3)
		sprintf(pinfo, "%s fullscreen", modelist[mode]);
	else
		sprintf(pinfo, "%s windowed", modelist[mode]);
	return pinfo;
}

void VID_AllocBuffers()
{
	// allocate z buffer and surface cache
	s32 chunk = vid.width * vid.height * sizeof(*d_pzbuffer);
	s32 cachesize = D_SurfaceCacheForRes(vid.width, vid.height);
	chunk += cachesize;
	if(litwater_base){
		D_FlushCaches();
		Hunk_FreeToHighMark(lwmark);
		litwater_base = NULL;
	}
	if(d_pzbuffer){
		D_FlushCaches();
		Hunk_FreeToHighMark(VID_highhunkmark);
		d_pzbuffer = NULL;
	}
	VID_highhunkmark = Hunk_HighMark();
	d_pzbuffer = Hunk_HighAllocName(chunk, "video");
	if(d_pzbuffer == NULL)
		Sys_Error("Not enough memory for video mode\n");
	u8 *cache = (u8 *) d_pzbuffer
	    + vid.width * vid.height * sizeof(*d_pzbuffer);
	D_InitCaches(cache, cachesize);
}

void VID_SetMode(s32 modenum, s32 custw, s32 custh, s32 custwinm, u8 */*palette*/)
{
	Con_DPrintf("SetMode: %d %dx%d %d\n", modenum, custw, custh, custwinm);
	if(modenum == vid_modenum && (!custw || !custh))
		return;
	if(custw && custh){
		vid.width = custw;
		vid.height = custh;
		vid_modenum = -1;
	} else {
		vid.width = oldmodes[modenum * 2];
		vid.height = oldmodes[modenum * 2 + 1];
		vid_modenum = modenum;
	}
	// CyanBun96: why do surfaces get freed but textures get destroyed :(
	SDL_FreeSurface(screen);
	SDL_FreeSurface(screentop);
	SDL_FreeSurface(screenui);
	SDL_FreeSurface(converted);
	screen = SDL_CreateRGBSurfaceWithFormat(0, vid.width, vid.height, 8,
		SDL_PIXELFORMAT_INDEX8);
	screentop = SDL_CreateRGBSurfaceWithFormat(0, vid.width, vid.height, 8,
		SDL_PIXELFORMAT_INDEX8);
	screenui = SDL_CreateRGBSurfaceWithFormat(0, vid.width, vid.height, 8,
		SDL_PIXELFORMAT_INDEX8);
	scrbuffs[0] = screen; scrbuffs[1] = screentop; scrbuffs[2] = screenui;
	if(!force_old_render){
		SDL_FreeSurface(argbbuffer);
		argbbuffer = SDL_CreateRGBSurfaceWithFormatFrom(NULL, vid.width,
			vid.height, 0, 0, SDL_PIXELFORMAT_ARGB8888);
		SDL_DestroyTexture(texture);
		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING, vid.width, vid.height);
	} else {
		SDL_FreeSurface(scaleBuffer);
		scaleBuffer = SDL_CreateRGBSurfaceWithFormat(0, vid.width,
			vid.height, 8, SDL_GetWindowPixelFormat (window));
	}
	vid.aspect = ((f32)vid.height / (f32)vid.width) * (320.0 / 240.0);
	vid.buffer = screen->pixels;
	VID_AllocBuffers();
	vid.recalc_refdef = 1;
	VID_SetPalette(worldpalname[0]?worldpal:host_basepal, screen);
	VID_SetPalette(uipalname[0]?uipal:host_basepal, screenui);
	SDL_SetColorKey(screenui, 1, 255);
	VID_SetPalette(host_basepal, screentop);
	SDL_SetColorKey(screentop, 1, 255);
	if(!custw || !custh){
		if(modenum <= 2){
			SDL_SetWindowFullscreen(window, 0);
			SDL_SetWindowSize(window,
				realwidth.value, realheight.value);
			SDL_SetWindowPosition(window, SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED);
		} else {
			SDL_SetWindowFullscreen(window,
				SDL_WINDOW_FULLSCREEN_DESKTOP);
		}
	} else {
		if(custwinm == 0){
			SDL_SetWindowFullscreen(window, 0);
			SDL_SetWindowSize(window,
					realwidth.value, realheight.value);
			SDL_SetWindowPosition(window, SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED);
		} else if(custwinm== 1){
			SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
		} else {
			SDL_SetWindowFullscreen(window,
				SDL_WINDOW_FULLSCREEN_DESKTOP);
		}
	}
	realwidth.value = 0;
	VID_CalcScreenDimensions(0);
	Cvar_SetValue("vid_mode", (f32)vid_modenum);
	Cvar_SetValue("vid_cwidth", (f32)custw);
	Cvar_SetValue("vid_cheight", (f32)custh);
	Cvar_SetValue("vid_cwmode", (f32)custwinm);
}

void VID_VidSetModeCommand_f()
{
	switch(Cmd_Argc()){
	default:
	case 1:
		Con_Printf("usage:\n");
		Con_Printf("   vid_setmode <width> <height> <mode>\n");
		Con_Printf("   modes: 0 - windowed\n");
		Con_Printf("          1 - fullscreen\n");
		Con_Printf("          2 - borderless\n");
		return;
	case 4:
		s32 custw = atoi(Cmd_Argv(1));
		s32 custh = atoi(Cmd_Argv(2));
		if(custw<320||custw>MAXWIDTH||custh<200||custh>MAXHEIGHT) {
			Con_Printf("320 <= width <= %d\n", MAXWIDTH);
			Con_Printf("200 <= height <= %d\n", MAXHEIGHT);
			return;
		}
		vid_realmode = -1;
		VID_SetMode(-1, custw, custh, atoi(Cmd_Argv(3)), vid_curpal);
		break;
	}
}
