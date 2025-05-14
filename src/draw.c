// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.
// this is the only file outside the refresh that touches the vid buffer
#include "quakedef.h"

static rectdesc_t r_rectdesc;

qpic_t *Draw_PicFromWad(s8 *name)
{
	return W_GetLumpName(name);
}

qpic_t *Draw_CachePic(s8 *path)
{
	cachepic_t *pic = menu_cachepics;
	s32 i = 0;
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


void Draw_CharacterScaled(s32 x, s32 y, s32 num, s32 scale)
{ // Draws one 8*8 graphics character with 0 being transparent.
  // It can be clipped to the top of the screen to allow the console
  // to be smoothly scrolled off.
	num &= 255;
	if (y <= -8)
		return; // totally off screen
	if (y > (s32)(vid.height - 8 * scale))
		return; // don't draw past the bottom of the screen
	s32 row = num >> 4;
	s32 col = num & 15;
	u8 *source = draw_chars + (row << 10) + (col << 3);
	s32 drawline;
	if (y < 0) { // clipped TODO doesn't seem to work properly
		drawline = 8 + y;
		source -= 128 * y;
		y = 0;
	} else
		drawline = 8;
	u8 *dest = drawlayer == 1 ? screen1->pixels : screen->pixels;
	dest += y * vid.width + x;
	while (drawline--) {
		for (s32 k = 0; k < scale; ++k) {
			for (s32 j = 0; j < scale; ++j) {
				for (s32 i = 0; i < 8; ++i)
					if (source[i])
						dest[i * scale + j] = source[i];
			}
			dest += vid.width;
		}
		source += 128;
	}
}

void Draw_StringScaled(s32 x, s32 y, s8 *str, s32 scale)
{
	while (*str) {
		Draw_CharacterScaled(x, y, *str, scale);
		str++;
		x += 8 * scale;
	}
}

void Draw_PicScaled(s32 x, s32 y, qpic_t *pic, s32 scale)
{
	u8 *dest = vid.buffer + y * vid.width + x;
	s32 maxwidth = pic->width;
	if (x + pic->width * scale > vid.width)
		maxwidth -= (x + pic->width * scale - vid.width) / scale;
	s32 rowinc = vid.width;
	if (y + pic->height * scale > vid.height)
		return;
	for (u32 v = 0; v < pic->height; v++) {
		if (v * scale + y >= vid.height)
			break;
		u8 *source = pic->data + v * pic->width;
		for (s32 k = 0; k < scale; k++) {
			u8 *dest_row = dest + k * rowinc;
			for (u32 i = 0; i < maxwidth; i++) {
				u8 pixel = source[i];
				for (s32 j = 0; j < scale; j++)
					dest_row[(i*scale) + j] = pixel;
			}
		}
		dest += rowinc * scale;
	}
}

void Draw_PicScaledPartial(s32 x,s32 y,s32 l,s32 t,s32 w,s32 h,qpic_t *p,s32 s)
{
        u8 *source = p->data;
        u8 *dest = vid.buffer + y * vid.width + x;
        for (u32 v = 0; v < p->height; v++) {
                if (v * s + y >= vid.height || v > h)
                        return;
                if (v < t)
                        continue;
                for (s32 k = 0; k < s; k++) {
                        for (u32 i = 0; i < p->width; i++) {
                                if (i < l || i >= w)
                                        continue;
                                for (s32 j = 0; j < s; j++)
                                        if (i * s + j + x < vid.width)
                                                dest[i * s + j] = source[i];
                        }
                        dest += vid.width;
                }
                source += p->width;
        }	
}

void Draw_TransPicScaled(s32 x, s32 y, qpic_t *pic, s32 scale)
{
	u8 *source = pic->data;
	u8 *dest = vid.buffer + y * vid.width + x;
	for (u32 v = 0; v < pic->height; v++) {
		if (v * scale + y >= vid.height)
			return;
		for (s32 k = 0; k < scale; k++) {
			for (u32 u = 0; u < pic->width; u++){
				u8 tbyte = source[u];
				if (tbyte == TRANSPARENT_COLOR)
					continue;
				for (s32 i = 0; i < scale; i++)
					if (u * scale + i + x < vid.width)
						dest[u * scale + i] = tbyte;
			}
			dest += vid.width;
		}
		source += pic->width;
	}
}

void Draw_TransPicTranslateScaled(s32 x, s32 y, qpic_t *p, u8 *tl, s32 scale)
{
	u8 *source = p->data;
	u8 *dest = vid.buffer + y * vid.width + x;
	for (u32 v = 0; v < p->height; v++) {
		if (v * scale + y >= vid.height)
			return;
		for (s32 k = 0; k < scale; k++) {
			for (u32 u = 0; u < p->width; u++) {
				u8 tbyte = source[u];
				if (tbyte == TRANSPARENT_COLOR)
					continue;
				for (s32 i = 0; i < scale; i++)
					if (u * scale + i + x < vid.width)
						dest[u * scale + i] = tl[tbyte];
			}
			dest += vid.width;
		}
		source += p->width;
	}
}

void Draw_CharToConback(s32 num, u8 *dest)
{
	s32 row = num >> 4;
	s32 col = num & 15;
	u8 *source = draw_chars + (row << 10) + (col << 3);
	s32 drawline = 8;
	while (drawline--) {
		for (s32 x = 0; x < 8; x++)
			if (source[x])
				dest[x] = 0x60 + source[x];
		source += 128;
		dest += 320;
	}
}

void Draw_CharToConbackScaled(s32 num, u8 *dest, s32 scale, s32 width)
{
	s32 row = num >> 4;
	s32 col = num & 15;
	u8 *source = draw_chars + (row << 10) + (col << 3);
	s32 drawline = 8;
	while (drawline--) {
		for (s32 x = 0; x < 8*scale; x++)
			if (source[x/scale])
				for (s32 s = 0; s < scale; s++)
					dest[x+s*width] = 0x60 + source[x/scale];
		source += 128;
		dest += width*scale;
	}
}

void Draw_ConsoleBackground(s32 lines)
{
	s8 ver[100];
	qpic_t *conback = Draw_CachePic("gfx/conback.lmp");
	// hack the version number directly into the pic
	sprintf(ver, "(QrustyQuake) %4.2f", (f32)VERSION);
	s32 scale = conback->width / 320;
	u8 *dest = conback->data + conback->width*(conback->height-14*scale)
		+ conback->width - 11*scale - 8*scale * strlen(ver);
	for (u64 x = 0; x < strlen(ver); x++)
		Draw_CharToConbackScaled(ver[x], dest + x * 8 * scale, scale, conback->width);
	dest = vid.buffer; // draw the pic
	for (s32 y = 0; y < lines; y++, dest += vid.width) {
		s32 v = (vid.height-lines+y)*conback->height/vid.height;
		u8 *src = conback->data + v * conback->width;
		if (vid.width == conback->width)
			memcpy(dest, src, vid.width);
		else {
			s32 f = 0;
			s32 fstep = conback->width * 0x10000 / vid.width;
			for (u32 x = 0; x < vid.width; x += 4) {
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

void R_DrawRect(vrect_t *prect, s32 rowbytes, u8 *psrc, s32 transparent)
{
	u8 *pdest = vid.buffer + (prect->y * vid.width) + prect->x;
	u64 maxdest = (u64)vid.buffer+vid.width*vid.height;
	if (transparent) {
		s32 srcdelta = rowbytes - prect->width;
		s32 destdelta = vid.width - prect->width;
		for (s32 i = 0; i < prect->height; i++) {
			for (s32 j = 0; j < prect->width; j++) {
				u8 t = *psrc;
				if (t != TRANSPARENT_COLOR)
					*pdest = t;
				psrc++;
				pdest++;
			}
			psrc += srcdelta;
			pdest += destdelta;
		}
	} else {
		for (s32 i = 0; i < prect->height; i++) {
			if ((u64)pdest+prect->width >= maxdest) break;
			memcpy(pdest, psrc, prect->width);
			psrc += rowbytes;
			pdest += vid.width;
		}
	}
}

void Draw_TileClear(s32 x, s32 y, s32 w, s32 h) // This repeats a 64*64
{ // tile graphic to fill the screen around a sized down refresh window
	r_rectdesc.rect.x = x;
	r_rectdesc.rect.y = y;
	r_rectdesc.rect.width = w;
	r_rectdesc.rect.height = h;
	vrect_t vr;
	vr.y = r_rectdesc.rect.y;
	s32 height = r_rectdesc.rect.height;
	s32 tileoffsety = vr.y % r_rectdesc.height;
	while (height > 0) {
		vr.x = r_rectdesc.rect.x;
		s32 width = r_rectdesc.rect.width;
		vr.height = r_rectdesc.height - tileoffsety;
		if (vr.height > height)
			vr.height = height;
		s32 tileoffsetx = vr.x % r_rectdesc.width;
		while (width > 0) {
			vr.width = r_rectdesc.width - tileoffsetx;
			if (vr.width > width)
				vr.width = width;
			u8 *psrc = r_rectdesc.ptexbytes +
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

void Draw_Fill(s32 x, s32 y, s32 w, s32 h, s32 c)
{ // Fills a box of pixels with a single color
	u8 *dest = vid.buffer + y * vid.width + x;
	for (s32 v = 0; v < h; v++, dest += vid.width)
		memset(dest, c, w); // Fast horizontal fill
}

void Draw_FadeScreen()
{
	for (u32 y = 0; y < vid.height / uiscale; y++)
		for (u32 i = 0; i < uiscale; i++) {
			u8 *pbuf = (u8 *) (vid.buffer + vid.width * y
				* uiscale + vid.width * i);
			u32 t = (y & 1) << 1;
			for (u32 x = 0; x < vid.width / uiscale; x++)
				if ((x & 3) != t)
					for (u32 j = 0; j<uiscale; j++)
						pbuf[x * uiscale + j] = 0;
		}
}
