// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

#include "quakedef.h"
#include "d_local.h"

extern int screenwidth;
static int miplevel;
float scale_for_mip;
int ubasestep, errorterm, erroradjustup, erroradjustdown;
int vstartscan;
vec3_t transformed_modelorg;
float winquake_surface_liquid_alpha;

// FIXME: should go away
extern void R_RotateBmodel();
extern void R_TransformFrustum();

int D_MipLevelForScale(float scale)
{
	int lmiplevel;
	if (scale >= d_scalemip[0])
		lmiplevel = 0;
	else if (scale >= d_scalemip[1])
		lmiplevel = 1;
	else if (scale >= d_scalemip[2])
		lmiplevel = 2;
	else
		lmiplevel = 3;
	if (lmiplevel < d_minmip)
		lmiplevel = d_minmip;
	return lmiplevel;
}

void D_DrawSolidSurface(surf_t *surf, int color)
{ // FIXME: clean this up
	int pix = (color << 24) | (color << 16) | (color << 8) | color;
	for (espan_t *span = surf->spans; span; span = span->pnext) {
		byte *pdest = (byte *) d_viewbuffer + screenwidth * span->v;
		int u = span->u;
		int u2 = span->u + span->count - 1;
		((byte *) pdest)[u] = pix;
		if (u2 - u < 8)
			for (u++; u <= u2; u++)
				((byte *) pdest)[u] = pix;
		else {
			for (u++; u & 3; u++)
				((byte *) pdest)[u] = pix;
			u2 -= 4;
			for (; u <= u2; u += 4)
				*(int *)((byte *) pdest + u) = pix;
			u2 += 4;
			for (; u <= u2; u++)
				((byte *) pdest)[u] = pix;
		}
	}
}

void D_CalcGradients(msurface_t *pface)
{
	float mipscale = 1.0 / (float)(1 << miplevel);
	vec3_t p_saxis, p_taxis;
	TransformVector(pface->texinfo->vecs[0], p_saxis);
	TransformVector(pface->texinfo->vecs[1], p_taxis);
	float t = xscaleinv * mipscale;
	d_sdivzstepu = p_saxis[0] * t;
	d_tdivzstepu = p_taxis[0] * t;
	t = yscaleinv * mipscale;
	d_sdivzstepv = -p_saxis[1] * t;
	d_tdivzstepv = -p_taxis[1] * t;
	d_sdivzorigin = p_saxis[2] * mipscale - xcenter * d_sdivzstepu -
						ycenter * d_sdivzstepv;
	d_tdivzorigin = p_taxis[2] * mipscale - xcenter * d_tdivzstepu -
						ycenter * d_tdivzstepv;
	vec3_t p_temp1;
	VectorScale(transformed_modelorg, mipscale, p_temp1);
	t = 0x10000 * mipscale;
	sadjust = ((fixed16_t) (DotProduct(p_temp1, p_saxis) * 0x10000 + 0.5)) -
				((pface->texturemins[0] << 16) >> miplevel)
				+ pface->texinfo->vecs[0][3] * t;
	tadjust = ((fixed16_t) (DotProduct(p_temp1, p_taxis) * 0x10000 + 0.5)) -
				((pface->texturemins[1] << 16) >> miplevel)
				+ pface->texinfo->vecs[1][3] * t;
	// -1 (-epsilon) so we never wander off the edge of the texture
	bbextents = ((pface->extents[0] << 16) >> miplevel) - 1;
	bbextentt = ((pface->extents[1] << 16) >> miplevel) - 1;
}

void D_DrawSurfaces()
{ // CyanBun96: TODO make this less huge, maybe split into several functions
	currententity = &cl_entities[0];
	TransformVector(modelorg, transformed_modelorg);
	vec3_t world_transformed_modelorg;
	VectorCopy(transformed_modelorg, world_transformed_modelorg);
	// TODO: could preset a lot of this at mode set time
	if (r_drawflat.value) {
		for (surf_t *s = &surfaces[1]; s < surface_p; s++) {
			if (!s->spans)
				continue;
			d_zistepu = s->d_zistepu;
			d_zistepv = s->d_zistepv;
			d_ziorigin = s->d_ziorigin;
			D_DrawSolidSurface(s, (uintptr_t) s->data & 0xFF);
			D_DrawZSpans(s->spans);
		}
	} else {
		for (surf_t *s = &surfaces[1]; s < surface_p; s++) {
			if (!s->spans)
				continue;
			r_drawnpolycount++;
			d_zistepu = s->d_zistepu;
			d_zistepv = s->d_zistepv;
			d_ziorigin = s->d_ziorigin;
			if (s->flags & SURF_DRAWSKY) {
				if (!r_skymade)
					R_MakeSky();
				D_DrawSkyScans8(s->spans);
				D_DrawZSpans(s->spans);
			} else if (s->flags & SURF_DRAWBACKGROUND) {
				// set up a gradient for the background surface that places it
				// effectively at infinity distance from the viewpoint
				d_zistepu = 0;
				d_zistepv = 0;
				d_ziorigin = -0.9;
				if (!r_pass || (int)r_twopass.value&1)
					D_DrawSolidSurface(s, (int)r_clearcolor.value & 0xFF);
				else D_DrawSolidSurface(s, 0xFF);
				D_DrawZSpans(s->spans);
			} else if (s->flags & SURF_DRAWTURB) {
				if ((s->flags & SURF_DRAWLAVA
					|| s->flags & SURF_DRAWSLIME
					|| s->flags & SURF_DRAWWATER)
					&& ((int)r_twopass.value&1 && !r_pass))
					continue;
				msurface_t *pface = s->data;
				miplevel = 0;
				cacheblock = (pixel_t *)
					((byte *) pface->texinfo->texture +
					pface->texinfo->texture->offsets[0]);
				cachewidth = 64;
				if (s->insubmodel) {
					// FIXME: we don't want to do all this for every polygon!
					// TODO: store once at start of frame
					currententity = s->entity; //FIXME: make this passed in to
					vec3_t local_modelorg;
					VectorSubtract(r_origin,
						       currententity->origin,
						       local_modelorg);
					TransformVector(local_modelorg,
							transformed_modelorg);
					R_RotateBmodel(); // FIXME: don't mess with the frustum,
					// make entity passed in
				}
				D_CalcGradients(pface);
				Turbulent8(s->spans);
				if ((int)r_twopass.value&1 && !r_pass) // Manoel Kasimier - translucent water
					D_DrawZSpans(s->spans);
				if (s->insubmodel) {
					// restore the old drawing state
					// FIXME: we don't want to do this every time!
					// TODO: speed up
					currententity = &cl_entities[0];
					VectorCopy(world_transformed_modelorg,
						   transformed_modelorg);
					VectorCopy(base_vpn, vpn);
					VectorCopy(base_vup, vup);
					VectorCopy(base_vright, vright);
					VectorCopy(base_modelorg, modelorg);
					R_TransformFrustum();
				}
			} else {
				if (s->insubmodel) {
					// FIXME: we don't want to do all this for every polygon!
					// TODO: store once at start of frame
					currententity = s->entity; //FIXME: make this passed in to
					vec3_t local_modelorg;
					VectorSubtract(r_origin, currententity
						->origin, local_modelorg);
					TransformVector(local_modelorg,
							transformed_modelorg);
					R_RotateBmodel(); // FIXME: don't mess with the frustum, make entity passed in
				}
				msurface_t *pface = s->data;
				miplevel = pface->flags & SURF_DRAWCUTOUT ? 0 :
					D_MipLevelForScale(s->nearzi *
						scale_for_mip *
						pface->texinfo->mipadjust);
				// FIXME: make this passed in to D_CacheSurface
				surfcache_t *pcurrentcache =
					D_CacheSurface(pface, miplevel);
				cacheblock = (pixel_t *) pcurrentcache->data;
				cachewidth = pcurrentcache->width;
				D_CalcGradients(pface);
				(*d_drawspans) (s->spans);
				D_DrawZSpans(s->spans);
				if (s->insubmodel) {
					// restore the old drawing state
					// FIXME: we don't want to do this every time!
					// TODO: speed up
					currententity = &cl_entities[0];
					VectorCopy(world_transformed_modelorg,
						   transformed_modelorg);
					VectorCopy(base_vpn, vpn);
					VectorCopy(base_vup, vup);
					VectorCopy(base_vright, vright);
					VectorCopy(base_modelorg, modelorg);
					R_TransformFrustum();
				}
			}
		}
	}
}
