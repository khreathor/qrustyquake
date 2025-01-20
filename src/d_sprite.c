// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

// software top-level rasterization driver module for drawing sprites

#include "quakedef.h"
#include "d_local.h"

static int sprite_height;
static int minindex, maxindex;
static sspan_t *sprite_spans;
sspan_t spans[MAXHEIGHT + 1];
extern int r_pass;

void D_SpriteDrawSpans(sspan_t *pspan)
{
	byte *pbase = cacheblock;
	float sdivz8stepu = d_sdivzstepu * 8;
	float tdivz8stepu = d_tdivzstepu * 8;
	float zi8stepu = d_zistepu * 8;
	// we count on FP exceptions being turned off to avoid range problems
	int izistep = (int)(d_zistepu * 0x8000 * 0x10000);
	do {
		byte *pdest = d_viewbuffer + (screenwidth*pspan->v) + pspan->u;
		short *pz = d_pzbuffer + (d_zwidth * pspan->v) + pspan->u;
		int count = pspan->count;
		if (count <= 0)
			goto NextSpan;
		// calculate the initial s/z, t/z, 1/z, s, and t and clamp
		float du = (float)pspan->u;
		float dv = (float)pspan->v;
		float sdivz = d_sdivzorigin+dv*d_sdivzstepv+du*d_sdivzstepu;
		float tdivz = d_tdivzorigin+dv*d_tdivzstepv+du*d_tdivzstepu;
		float zi = d_ziorigin + dv * d_zistepv + du * d_zistepu;
		float z = (float)0x10000 / zi; // prescale to 16.16 fixed-point
					       // we count on FP exceptions being off to avoid range problems
		int izi = (int)(zi * 0x8000 * 0x10000);
		fixed16_t s = (int)(sdivz * z) + sadjust;
		if (s > bbextents)
			s = bbextents;
		else if (s < 0)
			s = 0;
		fixed16_t t = (int)(tdivz * z) + tadjust;
		if (t > bbextentt)
			t = bbextentt;
		else if (t < 0)
			t = 0;
		do {
			// calculate s and t at the far end of the span
			int spancount = count >= 8 ? 8 : count;
			count -= spancount;
			fixed16_t sstep = 0, tstep = 0, snext = 0, tnext = 0;
			if (count) {
				// calculate s/z, t/z, zi->fixed s and t at far end of span,
				// calculate s and t steps across span by shifting
				sdivz += sdivz8stepu;
				tdivz += tdivz8stepu;
				zi += zi8stepu;
				z = (float)0x10000 / zi; // prescale to 16.16 fixed-point

				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8; // prevent round-off error on
						   // <0 steps from causing overstepping &
						   // running off the edge of the texture
				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8; // guard against round-off error on <0 steps
				sstep = (snext - s) >> 3;
				tstep = (tnext - t) >> 3;
			} else {
				// calculate s/z, t/z, zi->fixed s and t at last pixel in span (so
				// can't step off polygon), clamp, calculate s and t steps across
				// span by division, biasing steps low so we don't run off the
				// texture
				float spancountminus1 = (float)(spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (float)0x10000 / zi; // prescale to 16.16 fixed-point
				snext = (int)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8; // prevent round-off error on <0 steps from
						   // from causing overstepping & running off the
						   // edge of the texture
				tnext = (int)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8; // guard against round-off error on <0 steps
				if (spancount > 1) {
					sstep = (snext - s) / (spancount - 1);
					tstep = (tnext - t) / (spancount - 1);
				}
			}
			do {
				byte btemp = *(pbase + (s >> 16) +
						(t >> 16) * cachewidth);
				if (btemp != 255 && *pz <= (izi >> 16)) {
					*pz = izi >> 16;
					*pdest = btemp;
				}
				izi += izistep;
				pdest++;
				pz++;
				s += sstep;
				t += tstep;
			} while (--spancount > 0);
			s = snext;
			t = tnext;
		} while (count > 0);
NextSpan:
		pspan++;
	} while (pspan->count != DS_SPAN_LIST_END);
}

// CyanBun96: TODO these two look similar enough to combine, at least partially
void D_SpriteScanLeftEdge()
{ 
	sspan_t *pspan = sprite_spans;
	int i = minindex;
	if (i == 0)
		i = r_spritedesc.nump;
	int lmaxindex = maxindex;
	if (lmaxindex == 0)
		lmaxindex = r_spritedesc.nump;
	float vtop = ceil(r_spritedesc.pverts[i].v);
	do {
		emitpoint_t *pvert = &r_spritedesc.pverts[i];
		emitpoint_t *pnext = pvert - 1;
		float vbottom = ceil(pnext->v);
		if (vtop < vbottom) {
			float du = pnext->u - pvert->u;
			float dv = pnext->v - pvert->v;
			float slope = du / dv;
			fixed16_t u_step = (int)(slope * 0x10000);
			// adjust u to ceil the integer portion
			fixed16_t u = (int)((pvert->u + (slope *
				(vtop - pvert->v))) * 0x10000) + (0x10000 - 1);
			int itop = (int)vtop;
			int ibottom = (int)vbottom;
			for (int v = itop; v < ibottom; v++) {
				pspan->u = u >> 16;
				pspan->v = v;
				u += u_step;
				pspan++;
			}
		}
		vtop = vbottom;
		i--;
		if (i == 0)
			i = r_spritedesc.nump;
	} while (i != lmaxindex);
}

void D_SpriteScanRightEdge(void)
{
	sspan_t *pspan = sprite_spans;
	int i = minindex;
	float vvert = r_spritedesc.pverts[i].v;
	if (vvert < r_refdef.fvrecty_adj)
		vvert = r_refdef.fvrecty_adj;
	if (vvert > r_refdef.fvrectbottom_adj)
		vvert = r_refdef.fvrectbottom_adj;
	float vtop = ceil(vvert);
	do {
		emitpoint_t *pvert = &r_spritedesc.pverts[i];
		emitpoint_t *pnext = pvert + 1;
		float vnext = pnext->v;
		if (vnext < r_refdef.fvrecty_adj)
			vnext = r_refdef.fvrecty_adj;
		if (vnext > r_refdef.fvrectbottom_adj)
			vnext = r_refdef.fvrectbottom_adj;
		float vbottom = ceil(vnext);
		if (vtop < vbottom) {
			float uvert = pvert->u;
			if (uvert < r_refdef.fvrectx_adj)
				uvert = r_refdef.fvrectx_adj;
			if (uvert > r_refdef.fvrectright_adj)
				uvert = r_refdef.fvrectright_adj;
			float unext = pnext->u;
			if (unext < r_refdef.fvrectx_adj)
				unext = r_refdef.fvrectx_adj;
			if (unext > r_refdef.fvrectright_adj)
				unext = r_refdef.fvrectright_adj;
			float du = unext - uvert;
			float dv = vnext - vvert;
			float slope = du / dv;
			fixed16_t u_step = (int)(slope * 0x10000);
			// adjust u to ceil the integer portion
			fixed16_t u = (int)((uvert + (slope * (vtop - vvert)))
					* 0x10000)+ (0x10000 - 1);
			int itop = (int)vtop;
			int ibottom = (int)vbottom;
			for (int v = itop; v < ibottom; v++) {
				pspan->count = (u >> 16) - pspan->u;
				u += u_step;
				pspan++;
			}
		}
		vtop = vbottom;
		vvert = vnext;
		i++;
		if (i == r_spritedesc.nump)
			i = 0;
	} while (i != maxindex);
	pspan->count = DS_SPAN_LIST_END; // mark the end of the span list 
}

void D_SpriteCalculateGradients()
{
	vec3_t p_normal, p_saxis, p_taxis, p_temp1;
	TransformVector(r_spritedesc.vpn, p_normal);
	TransformVector(r_spritedesc.vright, p_saxis);
	TransformVector(r_spritedesc.vup, p_taxis);
	VectorInverse(p_taxis);
	float distinv = 1.0 / (-DotProduct(modelorg, r_spritedesc.vpn));
	d_sdivzstepu = p_saxis[0] * xscaleinv;
	d_tdivzstepu = p_taxis[0] * xscaleinv;
	d_sdivzstepv = -p_saxis[1] * yscaleinv;
	d_tdivzstepv = -p_taxis[1] * yscaleinv;
	d_zistepu = p_normal[0] * xscaleinv * distinv;
	d_zistepv = -p_normal[1] * yscaleinv * distinv;
	d_sdivzorigin = p_saxis[2]-xcenter*d_sdivzstepu-ycenter*d_sdivzstepv;
	d_tdivzorigin = p_taxis[2]-xcenter*d_tdivzstepu-ycenter*d_tdivzstepv;
	d_ziorigin = p_normal[2]*distinv-xcenter*d_zistepu-ycenter*d_zistepv;
	TransformVector(modelorg, p_temp1);
	sadjust = ((DotProduct(p_temp1, p_saxis) * 0x10000 + 0.5)) -
	    (-(cachewidth >> 1) << 16);
	tadjust = ((DotProduct(p_temp1, p_taxis) * 0x10000 + 0.5)) -
	    (-(sprite_height >> 1) << 16);
	// -1 (-epsilon) so we never wander off the edge of the texture
	bbextents = (cachewidth << 16) - 1;
	bbextentt = (sprite_height << 16) - 1;
}

void D_DrawSprite()
{
	sprite_spans = spans;
	float ymin = 999999.9; // find the top and bottom vertices, and make
	float ymax = -999999.9; // sure there's at least one scan to draw
	emitpoint_t *pverts = r_spritedesc.pverts;
	for (int i = 0; i < r_spritedesc.nump; i++) {
		if (pverts->v < ymin) {
			ymin = pverts->v;
			minindex = i;
		}
		if (pverts->v > ymax) {
			ymax = pverts->v;
			maxindex = i;
		}
		pverts++;
	}
	ymin = ceil(ymin);
	ymax = ceil(ymax);
	if (ymin >= ymax)
		return; // doesn't cross any scans at all
	cachewidth = r_spritedesc.pspriteframe->width;
	sprite_height = r_spritedesc.pspriteframe->height;
	cacheblock = (byte *) & r_spritedesc.pspriteframe->pixels[0];
	int nump = r_spritedesc.nump; // copy the first vertex to the last vertex,
	pverts = r_spritedesc.pverts; // so we don't have to deal with wrapping
	pverts[nump] = pverts[0];
	D_SpriteCalculateGradients();
	D_SpriteScanLeftEdge();
	D_SpriteScanRightEdge();
	D_SpriteDrawSpans(sprite_spans);
}
