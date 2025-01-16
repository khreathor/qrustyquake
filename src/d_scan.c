// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

// d_scan.c: Portable C scan-level rasterization code, all pixel depths.

#include "quakedef.h"
#include "r_local.h"
#include "d_local.h"

unsigned char *r_turb_pbase, *r_turb_pdest;
fixed16_t r_turb_s, r_turb_t, r_turb_sstep, r_turb_tstep;
int *r_turb_turb;
int r_turb_spancount;

void D_DrawTurbulent8Span();


void D_WarpScreen() // this performs a slight compression of the screen at the
{ // same time as the sine warp, to keep the edges from wrapping
	int w = r_refdef.vrect.width;
	int h = r_refdef.vrect.height;
	float wratio = w / (float)scr_vrect.width;
	float hratio = h / (float)scr_vrect.height;
	byte *rowptr[MAXHEIGHT + (AMP2 * 2)];
	int column[MAXWIDTH + (AMP2 * 2)];
	for (int v = 0; v < scr_vrect.height + AMP2 * 2; v++)
		rowptr[v] = d_viewbuffer + (r_refdef.vrect.y * screenwidth) +
		screenwidth * (int)((float)v * hratio * h / (h + AMP2 * 2));
	for (int u = 0; u < scr_vrect.width + AMP2 * 2; u++)
		column[u] = r_refdef.vrect.x + (int)((float)u * wratio * w /
			(w + AMP2 * 2));
	int *turb = intsintable + ((int)(cl.time * SPEED) & (CYCLE - 1));
	byte *dest = vid.buffer + scr_vrect.y * vid.rowbytes + scr_vrect.x;
	for (int v = 0; v < scr_vrect.height; v++, dest += vid.rowbytes) {
		int *col = &column[turb[v]];
		byte **row = &rowptr[v];
		for (int u = 0; u < scr_vrect.width; u += 4) {
			dest[u + 0] = row[turb[u + 0]][col[u + 0]];
			dest[u + 1] = row[turb[u + 1]][col[u + 1]];
			dest[u + 2] = row[turb[u + 2]][col[u + 2]];
			dest[u + 3] = row[turb[u + 3]][col[u + 3]];
		}
	}
}

void D_DrawTurbulent8Span()
{
	do {
		int s=((r_turb_s+r_turb_turb[(r_turb_t>>16)&(CYCLE-1)])>>16)&63;
		int t=((r_turb_t+r_turb_turb[(r_turb_s>>16)&(CYCLE-1)])>>16)&63;
		*r_turb_pdest++ = *(r_turb_pbase + (t<< 6) + s);
		r_turb_s += r_turb_sstep;
		r_turb_t += r_turb_tstep;
	} while (--r_turb_spancount > 0);
}

void Turbulent8(espan_t *pspan)
{
	r_turb_turb = sintable + ((int)(cl.time * SPEED) & (CYCLE - 1));
	r_turb_sstep = 0; // keep compiler happy
	r_turb_tstep = 0; // ditto
	r_turb_pbase = (unsigned char *)cacheblock;
	float sdivz16stepu = d_sdivzstepu * 16;
	float tdivz16stepu = d_tdivzstepu * 16;
	float zi16stepu = d_zistepu * 16;
	do {
		r_turb_pdest = (byte *) ((byte *) d_viewbuffer +
				(screenwidth * pspan->v) + pspan->u);
		int count = pspan->count;
		float du = (float)pspan->u; // calculate the initial s/z, t/z,
		float dv = (float)pspan->v; // 1/z, s, and t and clamp
		float sdivz = d_sdivzorigin+dv*d_sdivzstepv+du*d_sdivzstepu;
		float tdivz = d_tdivzorigin+dv*d_tdivzstepv+du*d_tdivzstepu;
		float zi = d_ziorigin + dv * d_zistepv + du * d_zistepu;
		float z = (float)0x10000 / zi; // prescale to 16.16 fixed-point
		r_turb_s = CLAMP(0, (int)(sdivz * z) + sadjust, bbextents);
		r_turb_t = CLAMP(0, (int)(tdivz * z) + tadjust, bbextentt);
		fixed16_t snxt, tnxt;
		do { // calculate s and t at the far end of the span
			r_turb_spancount = count >= 16 ? 16 : count;
			count -= r_turb_spancount;
			if (count) {
				// calculate s/z, t/z, zi->fixed s and t at far end of span,
				// calculate s and t steps across span by shifting
				sdivz += sdivz16stepu;
				tdivz += tdivz16stepu;
				zi += zi16stepu;
				z = (float)0x10000 / zi; // prescale to 16.16 fixed-point
				snxt=CLAMP(16,(int)(sdivz*z)+sadjust,bbextents);
				// prevent round-off error on <0 steps from causing overstepping
				// overstepping & running off the edge of the texture
				tnxt=CLAMP(16,(int)(tdivz*z)+tadjust,bbextentt);
				r_turb_sstep = (snxt - r_turb_s) >> 4;
				r_turb_tstep = (tnxt - r_turb_t) >> 4;
			} else {
				// calculate s/z, t/z, zi->fixed s and t at last pixel in span (so
				// can't step off polygon), clamp, calculate s and t steps across
				// span by division, biasing steps low so we don't run off the
				// texture
				float spancountminus1 = r_turb_spancount - 1;
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (float)0x10000 / zi; // prescale to 16.16 fixed-point
				snxt=CLAMP(16,(int)(sdivz*z)+sadjust,bbextents);
				// from causing overstepping & running off the
				// edge of the texture
				tnxt=CLAMP(16,(int)(tdivz*z)+tadjust,bbextentt);
				if (r_turb_spancount > 1) {
					r_turb_sstep = (snxt - r_turb_s)
						/ (r_turb_spancount - 1);
					r_turb_tstep = (tnxt - r_turb_t)
						/ (r_turb_spancount - 1);
				}
			}
			r_turb_s = r_turb_s & ((CYCLE << 16) - 1);
			r_turb_t = r_turb_t & ((CYCLE << 16) - 1);
			D_DrawTurbulent8Span();
			r_turb_s = snxt;
			r_turb_t = tnxt;
		} while (count > 0);
	} while ((pspan = pspan->pnext) != NULL);
}

void D_DrawSpans8(espan_t *pspan)
{
	fixed16_t sstep = 0, tstep = 0; // keep compiler happy
	unsigned char *pbase = (unsigned char *)cacheblock;
	float sdivz8stepu = d_sdivzstepu * 8;
	float tdivz8stepu = d_tdivzstepu * 8;
	float zi8stepu = d_zistepu * 8;
	do {
		unsigned char *pdest = (unsigned char *)((byte *) d_viewbuffer +
					(screenwidth * pspan->v) + pspan->u);
		int count = pspan->count;
		float du = (float)pspan->u; // calculate the initial s/z, t/z,
		float dv = (float)pspan->v; // 1/z, s, and t and clamp
		float sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		float tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		float zi = d_ziorigin + dv * d_zistepv + du * d_zistepu;
		float z = (float)0x10000 / zi; // prescale to 16.16 fixed-point
		fixed16_t s = CLAMP(0, (int)(sdivz * z) + sadjust, bbextents);
		fixed16_t t = CLAMP(0, (int)(tdivz * z) + tadjust, bbextentt);
		do { // calculate s and t at the far end of the span
			int spancount = (count >= 8) ? 8 : count;
			count -= spancount;
			fixed16_t snxt, tnxt;
			if (count) {
				// calculate s/z, t/z, zi->fixed s and t at far end of span,
				// calculate s and t steps across span by shifting
				sdivz += sdivz8stepu;
				tdivz += tdivz8stepu;
				zi += zi8stepu;
				z = (float)0x10000 / zi; // prescale to 16.16 fixed-point
				snxt=CLAMP(8,(int)(sdivz*z)+sadjust,bbextents);
				tnxt=CLAMP(8,(int)(tdivz*z)+tadjust,bbextentt);
				sstep = (snxt - s) >> 3;
				tstep = (tnxt - t) >> 3;
			} else {
				// calculate s/z, t/z, zi->fixed s and t at last pixel in span (so
				// can't step off polygon), clamp, calculate s and t steps across
				// span by division, biasing steps low so we don't run off the texture
				float spancountminus1 = (float)(spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (float)0x10000 / zi; // prescale to 16.16 fixed-point
				snxt=CLAMP(8,(int)(sdivz*z)+sadjust,bbextents);
				tnxt=CLAMP(8,(int)(tdivz*z)+tadjust,bbextents);
				if (spancount > 1) {
					sstep = (snxt - s) / (spancount - 1);
					tstep = (tnxt - t) / (spancount - 1);
				}
			}
			do {
				*pdest++ = *(pbase+(s>>16)+(t>>16)*cachewidth);
				s += sstep;
				t += tstep;
			} while (--spancount > 0);
			s = snxt;
			t = tnxt;
		} while (count > 0);
	} while ((pspan = pspan->pnext) != NULL);
}

void D_DrawZSpans(espan_t *pspan)
{
	int izistep = (int)(d_zistepu * 0x8000 * 0x10000);
	do {
		short *pdest = d_pzbuffer + (d_zwidth * pspan->v) + pspan->u;
		int count = pspan->count;
		float du = (float)pspan->u; // calculate the initial 1/z
		float dv = (float)pspan->v;
		double zi = d_ziorigin + dv * d_zistepv + du * d_zistepu;
		int izi = (int)(zi * 0x8000 * 0x10000);
		if ((intptr_t) pdest & 0x02) {
			*pdest++ = (short)(izi >> 16);
			izi += izistep;
			count--;
		}
		int doublecount = count >> 1;
		if (doublecount > 0) {
			do {
				unsigned int ltemp = izi >> 16;
				izi += izistep;
				ltemp |= izi & 0xFFFF0000;
				izi += izistep;
				*(int *)pdest = ltemp;
				pdest += 2;
			} while (--doublecount > 0);
		}
		if (count & 1)
			*pdest = (short)(izi >> 16);
	} while ((pspan = pspan->pnext) != NULL);
}
