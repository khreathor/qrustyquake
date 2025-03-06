// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

// this is the only file outside the refresh that touches the vid buffer
#include "quakedef.h"

#define	MAX_CACHED_PICS		128

typedef struct {
	vrect_t rect;
	int width;
	int height;
	byte *ptexbytes;
	int rowbytes;
} rectdesc_t;

typedef struct cachepic_s {
	char name[MAX_QPATH];
	cache_user_t cache;
} cachepic_t;

cachepic_t menu_cachepics[MAX_CACHED_PICS];
int menu_numcachepics;
static rectdesc_t r_rectdesc;
byte *draw_chars; // 8*8 graphic characters
qpic_t *draw_disc;
qpic_t *draw_backtile;

qpic_t *Draw_PicFromWad(char *name)
{
	return W_GetLumpName(name);
}

qpic_t *Draw_CachePic(char *path)
{
	cachepic_t *pic = menu_cachepics;
	int i = 0;
	for (; i < menu_numcachepics; pic++, i++)
		if (!strcmp(path, pic->name))
			break;
	if (i == menu_numcachepics) {
		if (menu_numcachepics == MAX_CACHED_PICS)
			Sys_Error("menu_numcachepics == MAX_CACHED_PICS");
		menu_numcachepics++;
		strcpy(pic->name, path);
	}
	qpic_t *dat = Cache_Check(&pic->cache);
	if (dat)
		return dat;
	COM_LoadCacheFile(path, &pic->cache, NULL); // load the pic from disk
	dat = (qpic_t *) pic->cache.data;
	if (!dat)
		Sys_Error("Draw_CachePic: failed to load %s", path);
	SwapPic(dat);
	return dat;
}

void Draw_Init()
{
	draw_chars = W_GetLumpName("conchars");
	draw_disc = W_GetLumpName("disc");
	draw_backtile = W_GetLumpName("backtile");
	r_rectdesc.width = draw_backtile->width;
	r_rectdesc.height = draw_backtile->height;
	r_rectdesc.ptexbytes = draw_backtile->data;
	r_rectdesc.rowbytes = draw_backtile->width;
}


void Draw_CharacterScaled(int x, int y, int num, int scale)
{ // Draws one 8*8 graphics character with 0 being transparent.
  // It can be clipped to the top of the screen to allow the console
  // to be smoothly scrolled off.
	num &= 255;
	if (y <= -8)
		return; // totally off screen
	if (y > (signed int)(vid.height - 8 * scale))
		return; // don't draw past the bottom of the screen
	int row = num >> 4;
	int col = num & 15;
	byte *source = draw_chars + (row << 10) + (col << 3);
	int drawline;
	if (y < 0) { // clipped TODO doesn't seem to work properly
		drawline = 8 + y;
		source -= 128 * y;
		y = 0;
	} else
		drawline = 8;
	byte *dest = vid.conbuffer + y * vid.conrowbytes + x;
	while (drawline--) {
		for (int k = 0; k < scale; ++k) {
			for (int j = 0; j < scale; ++j) {
				for (int i = 0; i < 8; ++i)
					if (source[i])
						dest[i * scale + j] = source[i];
			}
			dest += vid.conrowbytes;
		}
		source += 128;
	}
}

void Draw_StringScaled(int x, int y, char *str, int scale)
{
	while (*str) {
		Draw_CharacterScaled(x, y, *str, scale);
		str++;
		x += 8 * scale;
	}
}

void Draw_PicScaled(int x, int y, qpic_t *pic, int scale)
{
	byte *source = pic->data;
	byte *dest = vid.buffer + y * vid.rowbytes + x;
	for (unsigned int v = 0; v < pic->height; v++) {
		if (v * scale + y >= vid.height)
			return;
		for (int k = 0; k < scale; k++) {
			for (unsigned int i = 0; i < pic->width; i++) {
				for (int j = 0; j < scale; j++)
					if (i * scale + j + x < vid.width)
						dest[i * scale + j] = source[i];
			}
			dest += vid.rowbytes;
		}
		source += pic->width;
	}
}

void Draw_PicScaledPartial(int x, int y, int ox, int oy, int w, int h, qpic_t *pic, int scale)
{
	byte *source = pic->data;
	byte *dest = vid.buffer + y * vid.rowbytes + x;
	for (unsigned int v = 0; v < pic->height; v++) {
		if (v * scale + y >= vid.height || v > h)
			return;
		if (v < oy)
			continue;
		for (int k = 0; k < scale; k++) {
			for (unsigned int i = 0; i < pic->width; i++) {
				if (i < ox || i >= w)
					continue;
				for (int j = 0; j < scale; j++)
					if (i * scale + j + x < vid.width)
						dest[i * scale + j] = source[i];
			}
			dest += vid.rowbytes;
		}
		source += pic->width;
	}
}

void Draw_TransPicScaled(int x, int y, qpic_t *pic, int scale)
{
	byte *source = pic->data;
	byte *dest = vid.buffer + y * vid.rowbytes + x;
	byte tbyte;
	for (unsigned int v = 0; v < pic->height; v++) {
		if (v * scale + y >= vid.height)
			return;
		for (int k = 0; k < scale; k++) {
			for (unsigned int u = 0; u < pic->width; u++)
				if ((tbyte = source[u]) != TRANSPARENT_COLOR)
					for (int i = 0; i < scale; i++)
						if (u * scale + i + x < vid.width)
							dest[u * scale + i] = tbyte;

			dest += vid.rowbytes;
		}
		source += pic->width;
	}
}

void Draw_TransPicTranslateScaled(int x, int y, qpic_t *pic, byte *translation,
				  int scale)
{
	byte *source = pic->data;
	byte *dest = vid.buffer + y * vid.rowbytes + x;
	byte tbyte;
	for (unsigned int v = 0; v < pic->height; v++) {
		if (v * scale + y >= vid.height)
			return;
		for (int k = 0; k < scale; k++) {
			for (unsigned int u = 0; u < pic->width; u++)
				if ((tbyte = source[u]) != TRANSPARENT_COLOR)
					for (int i = 0; i < scale; i++)
						if (u * scale + i + x < vid.width)
							dest[u * scale + i] = translation[tbyte];
			dest += vid.rowbytes;
		}
		source += pic->width;
	}
}

void Draw_CharToConback(int num, byte *dest)
{
	int row = num >> 4;
	int col = num & 15;
	byte *source = draw_chars + (row << 10) + (col << 3);
	int drawline = 8;
	while (drawline--) {
		for (int x = 0; x < 8; x++)
			if (source[x])
				dest[x] = 0x60 + source[x];
		source += 128;
		dest += 320;
	}
}

void Draw_ConsoleBackground(int lines)
{
	char ver[100];
	qpic_t *conback = Draw_CachePic("gfx/conback.lmp");
	// hack the version number directly into the pic
	sprintf(ver, "(QrustyQuake) %4.2f", (float)VERSION);
	byte *dest = conback->data + 320 * 186 + 320 - 11 - 8 * strlen(ver);
	for (unsigned long x = 0; x < strlen(ver); x++)
		Draw_CharToConback(ver[x], dest + (x << 3));
	dest = vid.conbuffer; // draw the pic
	for (int y = 0; y < lines; y++, dest += vid.conrowbytes) {
		int v = (vid.conheight - lines + y) * 200 / vid.conheight;
		byte *src = conback->data + v * 320;
		if (vid.conwidth == 320)
			memcpy(dest, src, vid.conwidth);
		else {
			int f = 0;
			int fstep = 320 * 0x10000 / vid.conwidth;
			for (unsigned int x = 0; x < vid.conwidth; x += 4) {
				dest[x] = src[f >> 16];
				f += fstep;
				dest[x + 1] = src[f >> 16];
				f += fstep;
				dest[x + 2] = src[f >> 16];
				f += fstep;
				dest[x + 3] = src[f >> 16];
				f += fstep;
			}
		}
	}
}

void R_DrawRect(vrect_t *prect, int rowbytes, byte *psrc, int transparent)
{
	byte *pdest = vid.buffer + (prect->y * vid.rowbytes) + prect->x;
	if (transparent) {
		int srcdelta = rowbytes - prect->width;
		int destdelta = vid.rowbytes - prect->width;
		for (int i = 0; i < prect->height; i++) {
			for (int j = 0; j < prect->width; j++) {
				byte t = *psrc;
				if (t != TRANSPARENT_COLOR)
					*pdest = t;
				psrc++;
				pdest++;
			}
			psrc += srcdelta;
			pdest += destdelta;
		}
	} else {
		for (int i = 0; i < prect->height; i++) {
			memcpy(pdest, psrc, prect->width);
			psrc += rowbytes;
			pdest += vid.rowbytes;
		}
	}
}

void Draw_TileClear(int x, int y, int w, int h) // This repeats a 64*64
{ // tile graphic to fill the screen around a sized down refresh window
	r_rectdesc.rect.x = x;
	r_rectdesc.rect.y = y;
	r_rectdesc.rect.width = w;
	r_rectdesc.rect.height = h;
	vrect_t vr;
	vr.y = r_rectdesc.rect.y;
	int height = r_rectdesc.rect.height;
	int tileoffsety = vr.y % r_rectdesc.height;
	while (height > 0) {
		vr.x = r_rectdesc.rect.x;
		int width = r_rectdesc.rect.width;
		vr.height = r_rectdesc.height - tileoffsety;
		if (vr.height > height)
			vr.height = height;
		int tileoffsetx = vr.x % r_rectdesc.width;
		while (width > 0) {
			vr.width = r_rectdesc.width - tileoffsetx;
			if (vr.width > width)
				vr.width = width;
			byte *psrc = r_rectdesc.ptexbytes +
			    (tileoffsety * r_rectdesc.rowbytes) + tileoffsetx;
			R_DrawRect(&vr, r_rectdesc.rowbytes, psrc, 0);
			vr.x += vr.width;
			width -= vr.width;
			tileoffsetx = 0; // only the left tile can be left-clipped
		}
		vr.y += vr.height;
		height -= vr.height;
		tileoffsety = 0; // only the top tile can be top-clipped
	}
}

void Draw_Fill(int x, int y, int w, int h, int c)
{ // Fills a box of pixels with a single color
	byte *dest = vid.buffer + y * vid.rowbytes + x;
	for (int v = 0; v < h; v++, dest += vid.rowbytes)
		memset(dest, c, w); // Fast horizontal fill
}

void Draw_FadeScreen()
{
	for (unsigned int y = 0; y < vid.height / uiscale; y++)
		for (unsigned int i = 0; i < uiscale; i++) {
			byte *pbuf = (byte *) (vid.buffer + vid.rowbytes * y
				* uiscale + vid.rowbytes * i);
			unsigned int t = (y & 1) << 1;
			for (unsigned int x = 0; x < vid.width / uiscale; x++)
				if ((x & 3) != t)
					for (unsigned int j = 0; j<uiscale; j++)
						pbuf[x * uiscale + j] = 0;
		}
}
