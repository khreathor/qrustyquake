// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.
// d_scan.c: Portable C scan-level rasterization code, all pixel depths.
#include "quakedef.h"

static u8 *r_turb_pbase, *r_turb_pdest;
static s32 r_turb_s, r_turb_t, r_turb_sstep, r_turb_tstep;
static s32 *r_turb_turb;
static s32 r_turb_spancount;
static s16 *pz; // Manoel Kasimier - translucent water
static s32 izi, izistep;
static u8 cutoutbuf[MAXHEIGHT*MAXWIDTH];

void D_DrawTurbulent8Span();

void D_WarpScreen() // this performs a slight compression of the screen at the
{ // same time as the sine warp, to keep the edges from wrapping
	s32 w = r_refdef.vrect.width;
	s32 h = r_refdef.vrect.height;
	f32 wratio = w / (f32)scr_vrect.width;
	f32 hratio = h / (f32)scr_vrect.height;
	u8 *rowptr[MAXHEIGHT + (AMP2 * 2)];
	s32 column[MAXWIDTH + (AMP2 * 2)];
	for (s32 v = 0; v < scr_vrect.height + AMP2 * 2; v++)
		rowptr[v] = d_viewbuffer + (r_refdef.vrect.y * screenwidth) +
		    screenwidth * (s32)((f32)v * hratio * h / (h + AMP2 * 2));
	for (s32 u = 0; u < scr_vrect.width + AMP2 * 2; u++)
		column[u] = r_refdef.vrect.x + (s32)((f32)u * wratio * w /
						     (w + AMP2 * 2));
	s32 *turb = intsintable + ((s32)(cl.time * SPEED) & (CYCLE - 1));
	u8 *dest = vid.buffer + scr_vrect.y * vid.width + scr_vrect.x;
	for (s32 v = 0; v < scr_vrect.height; v++, dest += vid.width) {
		s32 *col = &column[turb[v]];
		u8 **row = &rowptr[v];
		for (s32 u = 0; u < scr_vrect.width; u += 4) {
			dest[u + 0] = row[turb[u + 0]][col[u + 0]];
			dest[u + 1] = row[turb[u + 1]][col[u + 1]];
			dest[u + 2] = row[turb[u + 2]][col[u + 2]];
			dest[u + 3] = row[turb[u + 3]][col[u + 3]];
		}
	}
}

void D_DrawTurbulent8Span()
{
	if (!lmonly) {
		do {
			s32 s = ((r_turb_s + r_turb_turb[(r_turb_t >> 16) & (CYCLE - 1)]) >> 16) & 63;
			s32 t = ((r_turb_t + r_turb_turb[(r_turb_s >> 16) & (CYCLE - 1)]) >> 16) & 63;
			s32 pix = *(r_turb_pbase + (t << 6) + s);
			*r_turb_pdest++ = pix;
			r_turb_s += r_turb_sstep;
			r_turb_t += r_turb_tstep;
		} while (--r_turb_spancount > 0);
	} else { // lit water: render first and then apply the already drawn lightmap as a filter
		if (!lit_lut_initialized) R_BuildLitLUT();
		do {
			s32 s = ((r_turb_s + r_turb_turb[(r_turb_t >> 16) & (CYCLE - 1)]) >> 16) & 63;
			s32 t = ((r_turb_t + r_turb_turb[(r_turb_s >> 16) & (CYCLE - 1)]) >> 16) & 63;
			s32 pix = *(r_turb_pbase + (t << 6) + s);
			s32 lit = *(litwater_base+(r_turb_pdest-d_viewbuffer));
			u8 rp = CURWORLDPAL[pix * 3 + 0];
			u8 gp = CURWORLDPAL[pix * 3 + 1];
			u8 bp = CURWORLDPAL[pix * 3 + 2];
			u8 rl = CURWORLDPAL[lit * 3 + 0];
			u8 gl = CURWORLDPAL[lit * 3 + 1];
			u8 bl = CURWORLDPAL[lit * 3 + 2];
			s32 r = rp * rl / 255;
			s32 g = gp * gl / 255;
			s32 b = bp * bl / 255;
			*r_turb_pdest++ = lit_lut[ QUANT(r)
                                        + QUANT(g)*LIT_LUT_RES
                                        + QUANT(b)*LIT_LUT_RES*LIT_LUT_RES ];
			r_turb_s += r_turb_sstep;
			r_turb_t += r_turb_tstep;
		} while (--r_turb_spancount > 0);
	}
}

void D_DrawTurbulent8SpanAlpha (f32 opacity)
{
	if (r_alphastyle.value == 0 && !lmonly) {
		if (!fog_lut_built) build_color_mix_lut(0);
		do {
			if (*pz <= (izi >> 16)) {
				s32 s=((r_turb_s+r_turb_turb[(r_turb_t>>16)&(CYCLE-1)])>>16)&63;
				s32 t=((r_turb_t+r_turb_turb[(r_turb_s>>16)&(CYCLE-1)])>>16)&63;
				*r_turb_pdest = color_mix_lut[*(r_turb_pbase + (t << 6) + s)]
					[*r_turb_pdest][(s32)(opacity*FOG_LUT_LEVELS)];
			}
			r_turb_pdest++;
			izi += izistep;
			pz++;
			r_turb_s += r_turb_sstep;
			r_turb_t += r_turb_tstep;
		} while (--r_turb_spancount > 0);
		return;
	} else if (r_alphastyle.value == 0 && lmonly) {
		if (!fog_lut_built) build_color_mix_lut(0);
		if (!lit_lut_initialized) R_BuildLitLUT();
		do {
			if (*pz <= (izi >> 16)) {
				s32 s=((r_turb_s+r_turb_turb[(r_turb_t>>16)&(CYCLE-1)])>>16)&63;
				s32 t=((r_turb_t+r_turb_turb[(r_turb_s>>16)&(CYCLE-1)])>>16)&63;
				s32 pix = *(r_turb_pbase + (t << 6) + s);
				s32 lit = *(litwater_base+(r_turb_pdest-d_viewbuffer));
				u8 rp = CURWORLDPAL[pix * 3 + 0];
				u8 gp = CURWORLDPAL[pix * 3 + 1];
				u8 bp = CURWORLDPAL[pix * 3 + 2];
				u8 rl = CURWORLDPAL[lit * 3 + 0];
				u8 gl = CURWORLDPAL[lit * 3 + 1];
				u8 bl = CURWORLDPAL[lit * 3 + 2];
				s32 r = rp * rl / 255;
				s32 g = gp * gl / 255;
				s32 b = bp * bl / 255;
				pix = lit_lut[ QUANT(r)
						+ QUANT(g)*LIT_LUT_RES
						+ QUANT(b)*LIT_LUT_RES*LIT_LUT_RES ];
				*r_turb_pdest = color_mix_lut[pix]
					[*r_turb_pdest][(s32)(opacity*FOG_LUT_LEVELS)];
			}
			r_turb_pdest++;
			izi += izistep;
			pz++;
			r_turb_s += r_turb_sstep;
			r_turb_t += r_turb_tstep;
		} while (--r_turb_spancount > 0);
		return;
	}
	if (!lmonly) {
		do {
			if (*pz <= (izi >> 16)) {
				s32 s=((r_turb_s+r_turb_turb[(r_turb_t>>16)&(CYCLE-1)])>>16)&63;
				s32 t=((r_turb_t+r_turb_turb[(r_turb_s>>16)&(CYCLE-1)])>>16)&63;
				if (D_Dither(r_turb_pdest, 1-opacity))
					*r_turb_pdest = *(r_turb_pbase + (t << 6) + s);
			}
			r_turb_pdest++;
			izi += izistep;
			pz++;
			r_turb_s += r_turb_sstep;
			r_turb_t += r_turb_tstep;
		} while (--r_turb_spancount > 0);
	} else {
		do {
			if (*pz <= (izi >> 16)) {
				s32 s=((r_turb_s+r_turb_turb[(r_turb_t>>16)&(CYCLE-1)])>>16)&63;
				s32 t=((r_turb_t+r_turb_turb[(r_turb_s>>16)&(CYCLE-1)])>>16)&63;
				if (D_Dither(r_turb_pdest, 1-opacity)) {
					s32 pix = *(r_turb_pbase + (t << 6) + s);
					s32 lit = *(litwater_base+(r_turb_pdest-d_viewbuffer));
					u8 rp = CURWORLDPAL[pix * 3 + 0];
					u8 gp = CURWORLDPAL[pix * 3 + 1];
					u8 bp = CURWORLDPAL[pix * 3 + 2];
					u8 rl = CURWORLDPAL[lit * 3 + 0];
					u8 gl = CURWORLDPAL[lit * 3 + 1];
					u8 bl = CURWORLDPAL[lit * 3 + 2];
					s32 r = rp * rl / 255;
					s32 g = gp * gl / 255;
					s32 b = bp * bl / 255;
					*r_turb_pdest = lit_lut[ QUANT(r)
							+ QUANT(g)*LIT_LUT_RES
							+ QUANT(b)*LIT_LUT_RES*LIT_LUT_RES ];
				}
			}
			r_turb_pdest++;
			izi += izistep;
			pz++;
			r_turb_s += r_turb_sstep;
			r_turb_t += r_turb_tstep;
		} while (--r_turb_spancount > 0);
	}
}

void Turbulent8(espan_t *pspan, f32 opacity)
{
	r_turb_turb = sintable + ((s32)(cl.time * SPEED) & (CYCLE - 1));
	r_turb_sstep = 0; // keep compiler happy
	r_turb_tstep = 0; // ditto
	r_turb_pbase = (u8 *)cacheblock;
	f32 sdivz16stepu = d_sdivzstepu * 16;
	f32 tdivz16stepu = d_tdivzstepu * 16;
	f32 zi16stepu = d_zistepu * 16;
	izistep = (s32)(d_zistepu * 0x8000 * 0x10000);
	do {
		r_turb_pdest = (u8 *) ((u8 *) d_viewbuffer + (screenwidth * pspan->v) + pspan->u);
		pz = d_pzbuffer + (d_zwidth * pspan->v) + pspan->u; // Manoel Kasimier - translucent water
		s32 count = pspan->count;
		f32 du = (f32)pspan->u; // calculate the initial s/z, t/z,
		f32 dv = (f32)pspan->v; // 1/z, s, and t and clamp
		f32 sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		f32 tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		f32 zi = d_ziorigin + dv * d_zistepv + du * d_zistepu;
		f32 z = (f32)0x10000 / zi; // prescale to 16.16 fixed-point
		izi = (s32)(zi * 0x8000 * 0x10000); // Manoel Kasimier - translucent water
		r_turb_s = (s32)(sdivz * z) + sadjust;
		if (r_turb_s > bbextents)
			r_turb_s = bbextents;
		else if (r_turb_s < 0)
			r_turb_s = 0;
		r_turb_t = (s32)(tdivz * z) + tadjust;
		if (r_turb_t > bbextentt)
			r_turb_t = bbextentt;
		else if (r_turb_t < 0)
			r_turb_t = 0;
		do {
			r_turb_spancount = count; // calculate s and t
			if (count >= 16) // at the far end of the span
				r_turb_spancount = 16;
			count -= r_turb_spancount;
			s32 snext, tnext;
			if (count) {
				// calculate s/z, t/z, zi->fixed s and t at far end of span,
				// calculate s and t steps across span by shifting
				sdivz += sdivz16stepu;
				tdivz += tdivz16stepu;
				zi += zi16stepu;
				z = (f32)0x10000 / zi; // prescale to 16.16 fixed-point
				snext = (s32)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 16)
					snext = 16; // prevent round-off error on <0 steps from
				// from causing overstepping & running off the
				// edge of the texture
				tnext = (s32)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 16)
					tnext = 16;
				// guard against round-off error on <0 steps
				r_turb_sstep = (snext - r_turb_s) >> 4;
				r_turb_tstep = (tnext - r_turb_t) >> 4;
			} else {
				// calculate s/z, t/z, zi->fixed s and t at last pixel in span (so
				// can't step off polygon), clamp, calculate s and t steps across
				// span by division, biasing steps low so we don't run off the
				// texture
				f32 spancountminus1 = (f32)(r_turb_spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (f32)0x10000 / zi; // prescale to 16.16 fixed-point
				snext = (s32)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 16)
					snext = 16; // prevent round-off error on <0 steps from
				// from causing overstepping & running off the
				// edge of the texture
				tnext = (s32)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 16)
					tnext = 16;// guard against round-off error on <0 steps
				if (r_turb_spancount > 1) {
					r_turb_sstep = (snext - r_turb_s) / (r_turb_spancount - 1);
					r_turb_tstep = (tnext - r_turb_t) / (r_turb_spancount - 1);
				}
			}
			r_turb_s = r_turb_s & ((CYCLE << 16) - 1);
			r_turb_t = r_turb_t & ((CYCLE << 16) - 1);
			if (r_wateralphapass)
				D_DrawTurbulent8SpanAlpha(opacity);
			else
				D_DrawTurbulent8Span();
			r_turb_s = snext;
			r_turb_t = tnext;
		} while (count > 0);
	} while ((pspan = pspan->pnext) != NULL);
}

void D_DrawSpans(espan_t *pspan, s32 type, f32 opacity)
{
	u8 *pbase = (u8 *)cacheblock;
	f32 sdivz8stepu = d_sdivzstepu * 8;
	f32 tdivz8stepu = d_tdivzstepu * 8;
	f32 zi8stepu = d_zistepu * 8;
	izistep = (s32)(d_zistepu * 0x8000 * 0x10000);
	do {
		u8 *pdest = (u8 *)((u8 *) d_viewbuffer +
				      (screenwidth * pspan->v) + pspan->u);
		if (lmonly) {
			if (!litwater_base) {
				lwmark = Hunk_HighMark();
				litwater_base = Hunk_HighAllocName(vid.width*vid.height, "waterlightmap");
			}
			pdest = (u8 *)((u8 *) litwater_base +
				      (screenwidth * pspan->v) + pspan->u);
		}
		s32 count = pspan->count;
		f32 du = (f32)pspan->u; // calculate the initial s/z, t/z,
		f32 dv = (f32)pspan->v; // 1/z, s, and t and clamp
		f32 sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		f32 tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		f32 zi = d_ziorigin + dv * d_zistepv + du * d_zistepu;
		f32 z = (f32)0x10000 / zi; // prescale to 16.16 fixed-point
		if (type == SPAN_TRANS || type == SPAN_CUTOUT) {
			pz = d_pzbuffer + (d_zwidth * pspan->v) + pspan->u;
			izi = (s32)(zi * 0x8000 * 0x10000);
		}
		s32 s = (s32)(sdivz * z) + sadjust;
		if (s > bbextents)
			s = bbextents;
		else if (s < 0)
			s = 0;
		s32 t = (s32)(tdivz * z) + tadjust;
		if (t > bbextentt)
			t = bbextentt;
		else if (t < 0)
			t = 0;
		do {
			// calculate s and t at the far end of the span
			s32 snext, tnext;
			s32 sstep = 0; // keep compiler happy
			s32 tstep = 0; // ditto
			s32 spancount = count;
			if (count >= 8)
				spancount = 8;
			count -= spancount;
			if (count) {
				// calculate s/z, t/z, zi->fixed s and t at far end of span,
				// calculate s and t steps across span by shifting
				sdivz += sdivz8stepu;
				tdivz += tdivz8stepu;
				zi += zi8stepu;
				z = (f32)0x10000 / zi; // prescale to 16.16 fixed-point
				snext = (s32)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8; // prevent round-off error on <0 steps from
				// from causing overstepping & running off the
				// edge of the texture
				tnext = (s32)(tdivz * z) + tadjust;
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
				f32 spancountminus1 = (f32)(spancount - 1);
				sdivz += d_sdivzstepu * spancountminus1;
				tdivz += d_tdivzstepu * spancountminus1;
				zi += d_zistepu * spancountminus1;
				z = (f32)0x10000 / zi; // prescale to 16.16 fixed-point
				snext = (s32)(sdivz * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8; // prevent round-off error on <0 steps from
				// from causing overstepping & running off the
				// edge of the texture
				tnext = (s32)(tdivz * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8; // guard against round-off error on <0 steps
				if (spancount > 1) {
					sstep = (snext - s) / (spancount - 1);
					tstep = (tnext - t) / (spancount - 1);
				}
			}
			if (type == SPAN_NORMAL) {
				do {
					u8 pix = *(pbase + (s >> 16) +
						(t >> 16) * cachewidth);
					*pdest = pix;
					pdest++;
					s += sstep;
					t += tstep;
				} while (--spancount > 0);
			}
			if (type == SPAN_CUTOUT) {
				do {
					if (*pz <= (izi >> 16)) {
						u8 pix = *(pbase + (s >> 16) +
							(t >> 16) * cachewidth);
						cutoutbuf[pdest-d_viewbuffer] = 0;
						if (pix != 0xff) {
							*pdest = pix;
							cutoutbuf[pdest-d_viewbuffer] = 1;
						}
					}
					pdest++;
					izi += izistep;
					pz++;
					s += sstep;
					t += tstep;
				} while (--spancount > 0);
			}
			else if (type == SPAN_SKYBOX) {
				s32 foglut = r_skyfog.value*FOG_LUT_LEVELS;
				do {
					u8 pix = *(pbase + (s >> 16) +
						(t >> 16) * cachewidth);
					if (fog_density > 0)
						pix = color_mix_lut[pix][fog_pal_index][foglut];
					*pdest = pix;
					pdest++;
					s += sstep;
					t += tstep;
				} while (--spancount > 0);
			}
			else if (type == SPAN_TRANS) {
				s32 foglut = opacity*FOG_LUT_LEVELS;
				if (r_alphastyle.value == 0) {
					if (!fog_lut_built)
						build_color_mix_lut(0);
					do {
						if (*pz <= (izi >> 16)) {
							u8 pix = *(pbase + (s >> 16) + (t >> 16) * cachewidth);
							if (pix != 0xff) {
								pix = color_mix_lut[pix][*pdest][foglut];
								*pdest = pix;
							}
						}
						pdest++;
						izi += izistep;
						pz++;
						s += sstep;
						t += tstep;
					} while (--spancount > 0);
				} else {
					do {
						if (*pz <= (izi >> 16) && D_Dither(pdest, 1-opacity)) {
							u8 pix = *(pbase + (s >> 16) + (t >> 16) * cachewidth);
							*pdest = pix;
						}
						pdest++;
						izi += izistep;
						pz++;
						s += sstep;
						t += tstep;
					} while (--spancount > 0);
				}
			}
			s = snext;
			t = tnext;
		} while (count > 0);
	} while ((pspan = pspan->pnext) != NULL);
}

void D_DrawZSpans(espan_t *pspan)
{
	s32 izistep = (s32)(d_zistepu * 0x8000 * 0x10000);
	do {
		s16 *pdest = d_pzbuffer + (d_zwidth * pspan->v) + pspan->u;
		s32 count = pspan->count;
		f32 du = (f32)pspan->u; // calculate the initial 1/z
		f32 dv = (f32)pspan->v;
		f64 zi = d_ziorigin + dv * d_zistepv + du * d_zistepu;
		s32 izi = (s32)(zi * 0x8000 * 0x10000);
		if ((intptr_t) pdest & 0x02) {
			*pdest++ = (s16)(izi >> 16);
			izi += izistep;
			count--;
		}
		s32 doublecount = count >> 1;
		if (doublecount > 0) {
			do {
				u32 ltemp = izi >> 16;
				izi += izistep;
				ltemp |= izi & 0xFFFF0000;
				izi += izistep;
				*(s32 *)pdest = ltemp;
				pdest += 2;
			} while (--doublecount > 0);
		}
		if (count & 1)
			*pdest = (s16)(izi >> 16);
	} while ((pspan = pspan->pnext) != NULL);
}

void D_DrawZSpansTrans(espan_t *pspan)
{
	s32 izistep = (s32)(d_zistepu * 0x8000 * 0x10000);
	do {
		s16 *pdest = d_pzbuffer + (d_zwidth * pspan->v) + pspan->u;
		s32 count = pspan->count;
		f32 du = (f32)pspan->u; // calculate the initial 1/z
		f32 dv = (f32)pspan->v;
		f64 zi = d_ziorigin + dv * d_zistepv + du * d_zistepu;
		s32 izi = (s32)(zi * 0x8000 * 0x10000);
		if ((intptr_t) pdest & 0x02) {
			if(cutoutbuf[(pdest-d_pzbuffer)] == 1 &&
				*pdest < (s16)(izi >> 16))
				*pdest++ = (s16)(izi >> 16);
			izi += izistep;
			count--;
		}
		s32 doublecount = count >> 1;
		if (doublecount > 0) {
			do {
				u32 ltemp = izi >> 16;
				izi += izistep;
				ltemp |= izi & 0xFFFF0000;
				izi += izistep;
				if(cutoutbuf[(pdest-d_pzbuffer)] == 1 &&
					*pdest < (s16)ltemp)
					*(s32 *)pdest = ltemp;
				pdest += 2;
			} while (--doublecount > 0);
		}
		if (count & 1) {
			if(cutoutbuf[(pdest-d_pzbuffer)] == 1 &&
				*pdest < (s16)(izi >> 16))
				*pdest = (s16)(izi >> 16);
		}
	} while ((pspan = pspan->pnext) != NULL);
}
