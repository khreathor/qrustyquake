// Copyright(c) 2024-2025 erysdren(it/its) GPLv3 See LICENSE for details.
// Originally MIT License
#include "quakedef.h"
#include "vgafont.h"

static const u8 palette[16][3] = { // ega 16-color palette
{0x00, 0x00, 0x00}, {0x00, 0x00, 0xab}, {0x00, 0xab, 0x00}, {0x00, 0xab, 0xab},
{0xab, 0x00, 0x00}, {0xab, 0x00, 0xab}, {0xab, 0x57, 0x00}, {0xab, 0xab, 0xab},
{0x57, 0x57, 0x57}, {0x57, 0x57, 0xff}, {0x57, 0xff, 0x57}, {0x57, 0xff, 0xff},
{0xff, 0x57, 0x57}, {0xff, 0x57, 0xff}, {0xff, 0xff, 0x57}, {0xff, 0xff, 0xff}
};

static void render_cell(u8 *image, s32 pitch, Uint16 cell, s32 noblink)
{
	u8 code = (u8)(cell & 0xFF); // get components
	u8 attr = (u8)(cell >> 8);
	u8 blink = (attr >> 7) & 0x01; // break out attributes
	u8 bgcolor = (attr >> 4) & 0x07;
	u8 fgcolor = attr & 0x0F;
	for(s32 y = 0; y < 16; y++){
		for(s32 x = 0; x < 8; x++){
			image[y * pitch + x] = bgcolor; // write bgcolor
			if(blink && noblink) continue;
			u8 *bitmap = &VGA_FONT_CP437[code*16]; // write fgcolor
			if(bitmap[y] & 1 << SDL_abs(x - 7))
				image[y * pitch + x] = fgcolor;
		}
	}
}

s32 vgatext_main(SDL_Window *window, Uint16 *screen)
{
	u8 image0[400][640];
	u8 image1[400][640];
	u32 format;
	SDL_Surface *surf8;
	SDL_Surface **windowsurf;
	SDL_Surface *windowsurf0;
	SDL_Surface *windowsurf1;
	if(!window || !screen) return -1;
	window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED,
			  SDL_WINDOWPOS_CENTERED, 640, 400, 0);
	format = SDL_PIXELFORMAT_INDEX8; // setup render surface
	surf8 = SDL_CreateRGBSurfaceWithFormat(0, 640, 400, 8, format);
	windowsurf0 = SDL_CreateRGBSurfaceWithFormat(0, 640, 400, 8, format);
        windowsurf1 = SDL_CreateRGBSurfaceWithFormat(0, 640, 400, 8, format);
        if(!windowsurf0 || !windowsurf1) return -1;
	if(!surf8) return -1;
	u8 *ppal = (u8 *)palette;
	SDL_Color pcolors[256];
        for(s32 i = 0; i < 256; ++i){
                pcolors[i].r = *ppal++;
                pcolors[i].g = *ppal++;
                pcolors[i].b = *ppal++;
        }
        SDL_SetPaletteColors(surf8->format->palette, pcolors, 0, 256);
	SDL_Rect rect = {0,0,640,400}; // setup blit rect
	for(s32 y = 0; y < 25; y++){ // render vgatext
		for(s32 x = 0; x < 80; x++){
			u16 cell = screen[y * 80 + x];
			u8 *imgpos0 = &image0[y * 16][x * 8];
			u8 *imgpos1 = &image1[y * 16][x * 8];
			render_cell(imgpos0, 640, cell, 0);
			render_cell(imgpos1, 640, cell, 1);
		}
	}
	for(s32 y = 0; y < 400; y++) // create rgb image0
		memcpy(&((u8*)surf8->pixels)[y*surf8->pitch],&image0[y][0],640);
	SDL_BlitSurface(surf8, &rect, windowsurf0, &rect);
	for(s32 y = 0; y < 400; y++) // create rgb image1
		memcpy(&((u8*)surf8->pixels)[y*surf8->pitch],&image1[y][0],640);
	SDL_BlitSurface(surf8, &rect, windowsurf1, &rect);
	windowsurf = &windowsurf0; // save windowsurface ptr
	u64 next = SDL_GetTicks() + BLINK_HZ; // start counting time
	while(1){ // main loop
		SDL_Event event;
		while(SDL_PollEvent(&event)){
			if(event.type == SDL_QUIT || event.type == SDL_KEYDOWN
					|| event.type == SDL_MOUSEBUTTONDOWN)
				goto done;
		}
		u64 now = SDL_GetTicks();
		if(next <= now){
			if(windowsurf == &windowsurf0)
				windowsurf = &windowsurf1;
			else windowsurf = &windowsurf0;
			next += BLINK_HZ;
		}
		SDL_BlitSurface(surf8,&rect,SDL_GetWindowSurface(window),&rect);
		SDL_UpdateWindowSurface(window);
	}
done:
	SDL_FreeSurface(surf8);
	SDL_FreeSurface(windowsurf0);
	SDL_FreeSurface(windowsurf1);
	return 0;
}
