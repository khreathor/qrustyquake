// Copyright (c) 2024-2025 erysdren (it/its) GPLv3 See LICENSE for details.
// Originally MIT License

#include "quakedef.h"
#include "vgafont.h"

// ega 16-color palette
static const Uint8 palette[16][3] = {
	{0x00, 0x00, 0x00}, {0x00, 0x00, 0xab}, {0x00, 0xab, 0x00}, {0x00, 0xab, 0xab},
	{0xab, 0x00, 0x00}, {0xab, 0x00, 0xab}, {0xab, 0x57, 0x00}, {0xab, 0xab, 0xab},
	{0x57, 0x57, 0x57}, {0x57, 0x57, 0xff}, {0x57, 0xff, 0x57}, {0x57, 0xff, 0xff},
	{0xff, 0x57, 0x57}, {0xff, 0x57, 0xff}, {0xff, 0xff, 0x57}, {0xff, 0xff, 0xff}
};

static void render_cell(Uint8 *image, int pitch, Uint16 cell, int noblink)
{
	// get components
	Uint8 code = (Uint8)(cell & 0xFF);
	Uint8 attr = (Uint8)(cell >> 8);

	// break out attributes
	Uint8 blink = (attr >> 7) & 0x01;
	Uint8 bgcolor = (attr >> 4) & 0x07;
	Uint8 fgcolor = attr & 0x0F;

	for (int y = 0; y < 16; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			// write bgcolor
			image[y * pitch + x] = bgcolor;

			if (blink && noblink)
				continue;

			// write fgcolor
			Uint8 *bitmap = &VGA_FONT_CP437[code * 16];

			if (bitmap[y] & 1 << SDL_abs(x - 7))
				image[y * pitch + x] = fgcolor;
		}
	}
}

int vgatext_main(SDL_Window *window, Uint16 *screen)
{
	Uint8 image0[400][640];
	Uint8 image1[400][640];
	Uint32 format;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	SDL_Surface *surface8;
	SDL_Surface **windowsurface;
	SDL_Surface *windowsurface0;
	SDL_Surface *windowsurface1;
	SDL_Color colors[16];
	SDL_Rect rect;

	if (!window || !screen)
		return -1;

	window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED,
			  SDL_WINDOWPOS_CENTERED,
			  640, 400, 0);

	// setup render surface
	format = SDL_PIXELFORMAT_INDEX8;
	surface8 = SDL_CreateRGBSurfaceWithFormat(0, 640, 400, 8, format);
	windowsurface0 = SDL_CreateRGBSurfaceWithFormat(0, 640, 400, 8, format);
        windowsurface1 = SDL_CreateRGBSurfaceWithFormat(0, 640, 400, 8, format);
        if (!windowsurface0 || !windowsurface1)
                return -1;
	if (!surface8)
		return -1;

	unsigned char *ppal = (unsigned char *)palette;
	SDL_Color pcolors[256];
        for (int i = 0; i < 256; ++i) {
                pcolors[i].r = *ppal++;
                pcolors[i].g = *ppal++;
                pcolors[i].b = *ppal++;
        }
        SDL_SetPaletteColors(surface8->format->palette, pcolors, 0, 256);

	// setup blit rect
	rect.x = 0;
	rect.y = 0;
	rect.w = 640;
	rect.h = 400;

	// render vgatext
	for (int y = 0; y < 25; y++)
	{
		for (int x = 0; x < 80; x++)
		{
			uint16_t cell = screen[y * 80 + x];
			uint8_t *imgpos0 = &image0[y * 16][x * 8];
			uint8_t *imgpos1 = &image1[y * 16][x * 8];

			render_cell(imgpos0, 640, cell, 0);
			render_cell(imgpos1, 640, cell, 1);
		}
	}

	// create rgb image0
	for (int y = 0; y < 400; y++)
		memcpy(&((Uint8 *)surface8->pixels)[y * surface8->pitch], &image0[y][0], 640);
	SDL_BlitSurface(surface8, &rect, windowsurface0, &rect);

	// create rgb image1
	for (int y = 0; y < 400; y++)
		memcpy(&((Uint8 *)surface8->pixels)[y * surface8->pitch], &image1[y][0], 640);
	SDL_BlitSurface(surface8, &rect, windowsurface1, &rect);

	// save windowsurface ptr
	windowsurface = &windowsurface0;

	// start counting time
	Uint64 next = SDL_GetTicks() + BLINK_HZ;

	// main loop
	while (1)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT || event.type == SDL_KEYDOWN || event.type == SDL_MOUSEBUTTONDOWN)
				goto done;
		}

		Uint64 now = SDL_GetTicks();
		if (next <= now)
		{
			if (windowsurface == &windowsurface0)
				windowsurface = &windowsurface1;
			else
				windowsurface = &windowsurface0;

			next += BLINK_HZ;
		}

		SDL_BlitSurface(surface8, &rect, SDL_GetWindowSurface(window), &rect);
		SDL_UpdateWindowSurface(window);
	}

done:

	//SDL_FreeSurface(surface8);
	//SDL_FreeSurface(windowsurface0);
	//SDL_FreeSurface(windowsurface1);
	//SDL_DestroyTexture(texture);
	//SDL_DestroyRenderer(renderer);

	return 0;
}
