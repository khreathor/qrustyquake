// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

// this is the only file outside the refresh that touches the vid buffer
#include "quakedef.h"

cachepic_t menu_cachepics[MAX_CACHED_PICS];
int menu_numcachepics;
static rectdesc_t r_rectdesc;
byte *draw_chars; // 8*8 graphic characters
qpic_t *draw_disc;
qpic_t *draw_backtile;
int drawlayer = 0;

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
	byte *dest = drawlayer == 1 ? screen1->pixels : screen->pixels;
	dest += y * vid.width + x;
	while (drawline--) {
		for (int k = 0; k < scale; ++k) {
			for (int j = 0; j < scale; ++j) {
				for (int i = 0; i < 8; ++i)
					if (source[i])
						dest[i * scale + j] = source[i];
			}
			dest += vid.width;
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
	byte *dest = vid.buffer + y * vid.width + x;
	int maxwidth = pic->width;
	if (x + pic->width * scale > vid.width)
		maxwidth -= (x + pic->width * scale - vid.width) / scale;
	int rowinc = vid.width;
	if (y + pic->height * scale > vid.height)
		return;
	for (unsigned int v = 0; v < pic->height; v++) {
		if (v * scale + y >= vid.height)
			break;
		byte *source = pic->data + v * pic->width;
		for (int k = 0; k < scale; k++) {
			byte *dest_row = dest + k * rowinc;
			for (unsigned int i = 0; i < maxwidth; i++) {
				byte pixel = source[i];
				for (int j = 0; j < scale; j++)
					dest_row[(i*scale) + j] = pixel;
			}
		}
		dest += rowinc * scale;
	}
}

void Draw_PicScaledPartial(int x,int y,int l,int t,int w,int h,qpic_t *p,int s)
{
        byte *source = p->data;
        byte *dest = vid.buffer + y * vid.width + x;
        for (unsigned int v = 0; v < p->height; v++) {
                if (v * s + y >= vid.height || v > h)
                        return;
                if (v < t)
                        continue;
                for (int k = 0; k < s; k++) {
                        for (unsigned int i = 0; i < p->width; i++) {
                                if (i < l || i >= w)
                                        continue;
                                for (int j = 0; j < s; j++)
                                        if (i * s + j + x < vid.width)
                                                dest[i * s + j] = source[i];
                        }
                        dest += vid.width;
                }
                source += p->width;
        }	
}

void Draw_TransPicScaled(int x, int y, qpic_t *pic, int scale)
{
	byte *source = pic->data;
	byte *dest = vid.buffer + y * vid.width + x;
	for (unsigned int v = 0; v < pic->height; v++) {
		if (v * scale + y >= vid.height)
			return;
		for (int k = 0; k < scale; k++) {
			for (unsigned int u = 0; u < pic->width; u++){
				byte tbyte = source[u];
				if (tbyte == TRANSPARENT_COLOR)
					continue;
				for (int i = 0; i < scale; i++)
					if (u * scale + i + x < vid.width)
						dest[u * scale + i] = tbyte;
			}
			dest += vid.width;
		}
		source += pic->width;
	}
}

void Draw_TransPicTranslateScaled(int x, int y, qpic_t *p, byte *tl, int scale)
{
	byte *source = p->data;
	byte *dest = vid.buffer + y * vid.width + x;
	for (unsigned int v = 0; v < p->height; v++) {
		if (v * scale + y >= vid.height)
			return;
		for (int k = 0; k < scale; k++) {
			for (unsigned int u = 0; u < p->width; u++) {
				byte tbyte = source[u];
				if (tbyte == TRANSPARENT_COLOR)
					continue;
				for (int i = 0; i < scale; i++)
					if (u * scale + i + x < vid.width)
						dest[u * scale + i] = tl[tbyte];
			}
			dest += vid.width;
		}
		source += p->width;
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

void Draw_CharToConbackScaled(int num, byte *dest, int scale, int width)
{
	int row = num >> 4;
	int col = num & 15;
	byte *source = draw_chars + (row << 10) + (col << 3);
	int drawline = 8;
	while (drawline--) {
		for (int x = 0; x < 8*scale; x++)
			if (source[x/scale])
				for (int s = 0; s < scale; s++)
					dest[x+s*width] = 0x60 + source[x/scale];
		source += 128;
		dest += width*scale;
	}
}

void Draw_ConsoleBackground(int lines)
{
	char ver[100];
	qpic_t *conback = Draw_CachePic("gfx/conback.lmp");
	// hack the version number directly into the pic
	sprintf(ver, "(QrustyQuake) %4.2f", (float)VERSION);
	int scale = conback->width / 320;
	byte *dest = conback->data + conback->width*(conback->height-14*scale)
		+ conback->width - 11*scale - 8*scale * strlen(ver);
	for (unsigned long x = 0; x < strlen(ver); x++)
		Draw_CharToConbackScaled(ver[x], dest + x * 8 * scale, scale, conback->width);
	dest = vid.buffer; // draw the pic
	for (int y = 0; y < lines; y++, dest += vid.width) {
		int v = (vid.height-lines+y)*conback->height/vid.height;
		byte *src = conback->data + v * conback->width;
		if (vid.width == conback->width)
			memcpy(dest, src, vid.width);
		else {
			int f = 0;
			int fstep = conback->width * 0x10000 / vid.width;
			for (unsigned int x = 0; x < vid.width; x += 4) {
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
	byte *pdest = vid.buffer + (prect->y * vid.width) + prect->x;
	unsigned long maxdest = (unsigned long)vid.buffer+vid.width*vid.height;
	if (transparent) {
		int srcdelta = rowbytes - prect->width;
		int destdelta = vid.width - prect->width;
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
			if ((unsigned long)pdest+prect->width >= maxdest) break;
			memcpy(pdest, psrc, prect->width);
			psrc += rowbytes;
			pdest += vid.width;
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
			tileoffsetx = 0; // only left tile can be left-clipped
		}
		vr.y += vr.height;
		height -= vr.height;
		tileoffsety = 0; // only top tile can be top-clipped
	}
}

void Draw_Fill(int x, int y, int w, int h, int c)
{ // Fills a box of pixels with a single color
	byte *dest = vid.buffer + y * vid.width + x;
	for (int v = 0; v < h; v++, dest += vid.width)
		memset(dest, c, w); // Fast horizontal fill
}

void Draw_FadeScreen()
{
	for (unsigned int y = 0; y < vid.height / uiscale; y++)
		for (unsigned int i = 0; i < uiscale; i++) {
			byte *pbuf = (byte *) (vid.buffer + vid.width * y
				* uiscale + vid.width * i);
			unsigned int t = (y & 1) << 1;
			for (unsigned int x = 0; x < vid.width / uiscale; x++)
				if ((x & 3) != t)
					for (unsigned int j = 0; j<uiscale; j++)
						pbuf[x * uiscale + j] = 0;
		}
}
