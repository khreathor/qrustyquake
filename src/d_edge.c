// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.
#include "quakedef.h"

static s32 miplevel;

s32 D_MipLevelForScale(f32 scale)
{
	s32 lmiplevel;
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

void D_DrawSolidSurface(surf_t *surf, s32 color)
{ // FIXME: clean this up
	s32 pix = (color << 24) | (color << 16) | (color << 8) | color;
	for (espan_t *span = surf->spans; span; span = span->pnext) {
		u8 *pdest = (u8 *) d_viewbuffer + screenwidth * span->v;
		s32 u = span->u;
		s32 u2 = span->u + span->count - 1;
		((u8 *) pdest)[u] = pix;
		if (u2 - u < 8)
			for (u++; u <= u2; u++)
				((u8 *) pdest)[u] = pix;
		else {
			for (u++; u & 3; u++)
				((u8 *) pdest)[u] = pix;
			u2 -= 4;
			for (; u <= u2; u += 4)
				*(s32 *)((u8 *) pdest + u) = pix;
			u2 += 4;
			for (; u <= u2; u++)
				((u8 *) pdest)[u] = pix;
		}
	}
}

void D_CalcGradients(msurface_t *pface)
{
	f32 mipscale = 1.0 / (f32)(1 << miplevel);
	vec3_t p_saxis, p_taxis;
	TransformVector(pface->texinfo->vecs[0], p_saxis);
	TransformVector(pface->texinfo->vecs[1], p_taxis);
	f32 t = xscaleinv * mipscale;
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
	sadjust = ((s32) (DotProduct(p_temp1, p_saxis) * 0x10000 + 0.5)) -
				((pface->texturemins[0] << 16) >> miplevel)
				+ pface->texinfo->vecs[0][3] * t;
	tadjust = ((s32) (DotProduct(p_temp1, p_taxis) * 0x10000 + 0.5)) -
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
		return;
	}
	for (surf_t *s = &surfaces[1]; s < surface_p; s++) {
		if (!s->spans)
			continue;
		r_drawnpolycount++;
		msurface_t *pface = s->data;
		if (pface == 0) continue;
		// CyanBun96: some entities are assigned an invalid address like
		// 35, which leads to segfaults on any further checks while
		// still passing s->entity != NULL check. Must be a symptom of
		// some bigger issue that I can't be bothered to diagnose ATM.
		u64 is_ent = (u64)s->entity & 0xffff000;
		// CyanBun96: a 0 in either of those causes an error. FIXME
		if (!pface || !pface->extents[0] || !pface->extents[1]) {
			if (pface)
				Con_DPrintf("Broken surface extents %hd %hd\n",
					pface->extents[0], pface->extents[1]);
			else
				Con_DPrintf("Broken surface\n");
			continue;
		}
		// CyanBun96: reject broken surfaces earlier to avoid crashes
		s32 wd = pface->extents[0] >> (MIPLEVELS-1);
		s32 sz = wd * pface->extents[1] >> (MIPLEVELS-1);
		if (((wd < 0 || wd > 256) || (sz <= 0 || sz > 0x10000)) &&
			!SURF_WINQUAKE_DRAWTRANSLUCENT) {
			Con_DPrintf("Invalid surface width/size %d %d\n",wd,sz);
			continue;
		}
		if (is_ent && s->entity->alpha && r_entalpha.value == 1)
			winquake_surface_liquid_alpha = (f32)s->entity->alpha / 255;
		// Baker: Need to determine what kind of liquid we are
		else if (s->flags & SURF_WINQUAKE_DRAWTRANSLUCENT) {
			if (s->flags & SURF_DRAWLAVA)
				winquake_surface_liquid_alpha = R_WaterAlphaForTextureType(TEXTYPE_LAVA);
			else if (s->flags & SURF_DRAWSLIME)
				winquake_surface_liquid_alpha = R_WaterAlphaForTextureType(TEXTYPE_SLIME);
			else if (s->flags & SURF_DRAWWATER)
				winquake_surface_liquid_alpha = R_WaterAlphaForTextureType(TEXTYPE_WATER);
			else if (s->flags & SURF_DRAWTELE)
				winquake_surface_liquid_alpha = R_WaterAlphaForTextureType(TEXTYPE_TELE);
		} else
			winquake_surface_liquid_alpha = 1;
		if (r_wateralphapass && winquake_surface_liquid_alpha == 1 && r_entalpha.value == 1)
			continue; // Manoel Kasimier - translucent water
		d_zistepu = s->d_zistepu;
		d_zistepv = s->d_zistepv;
		d_ziorigin = s->d_ziorigin;
		lmonly = 0;
		if (s->flags & SURF_DRAWSKY) {
			if (!r_skymade)
				R_MakeSky();
			D_DrawSkyScans8(s->spans);
			D_DrawZSpans(s->spans);
		} else if (s->flags & SURF_DRAWSKYBOX) {
			// Manoel Kasimier - hi-res skyboxes - edited
			extern u8 r_skypixels[6][SKYBOX_MAX_SIZE*SKYBOX_MAX_SIZE];
			miplevel = 0;
			cacheblock = (u8 *)(r_skypixels[pface->texinfo->texture->offsets[0]]);
			// Manoel Kasimier - hi-res skyboxes - edited
			cachewidth = pface->texinfo->texture->width;
			d_zistepu = s->d_zistepu;
			d_zistepv = s->d_zistepv;
			d_ziorigin = s->d_ziorigin;
			D_CalcGradients (pface);
			D_DrawSkyboxScans8(s->spans);
			// set up a gradient for the background surface that places it
			// effectively at infinity distance from the viewpoint
			d_zistepu = 0;
			d_zistepv = 0;
			d_ziorigin = -0.9;
			D_DrawZSpans (s->spans);
		} else if (s->flags & SURF_DRAWBACKGROUND) {
			// set up a gradient for the background surface that places it
			// effectively at infinity distance from the viewpoint
			if (skybox_name[0] || r_wateralphapass)
				continue;
			d_zistepu = 0;
			d_zistepv = 0;
			d_ziorigin = -0.9;
			if (!r_pass || (s32)r_twopass.value&1)
				D_DrawSolidSurface(s, (s32)r_clearcolor.value & 0xFF);
			else D_DrawSolidSurface(s, 0xFF);
			D_DrawZSpans(s->spans);
		} else if (s->flags & SURF_DRAWTURB && (!s->entity->model->haslitwater
				|| !r_litwater.value)) {
			miplevel = 0;
			cacheblock = (u8 *)
				((u8 *) pface->texinfo->texture +
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
			f32 opacity = 1;
			if (s->entity && s->entity->alpha && r_entalpha.value == 1)
				opacity -= (f32)s->entity->alpha/255;
			else if (s->flags & SURF_DRAWLAVA) opacity -= R_WaterAlphaForTextureType(TEXTYPE_LAVA);
			else if (s->flags & SURF_DRAWSLIME) opacity -= R_WaterAlphaForTextureType(TEXTYPE_SLIME);
			else if (s->flags & SURF_DRAWWATER) opacity -= R_WaterAlphaForTextureType(TEXTYPE_WATER);
			else if (s->flags & SURF_DRAWTELE) opacity -= R_WaterAlphaForTextureType(TEXTYPE_TELE);
			Turbulent8(s->spans, opacity);
			if (!r_wateralphapass) // Manoel Kasimier - translucent water
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
		} else if (is_ent && s->entity->alpha && r_entalpha.value == 1) {
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
			miplevel = pface->flags & SURF_DRAWCUTOUT ? 0 :
				D_MipLevelForScale(s->nearzi *
					scale_for_mip *
					pface->texinfo->mipadjust);
			// FIXME: make this passed in to D_CacheSurface
			surfcache_t *pcurrentcache =
				D_CacheSurface(pface, miplevel);
			cacheblock = (u8 *) pcurrentcache->data;
			cachewidth = pcurrentcache->width;
			D_CalcGradients(pface);
			f32 opacity = 1 - (f32)s->entity->alpha / 255;
			D_DrawTransSpans8(s->spans, opacity);
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
		} else if (s->flags & SURF_DRAWTURB && s->entity->model->haslitwater && r_litwater.value) {
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
			miplevel = pface->flags & SURF_DRAWCUTOUT ? 0 :
				D_MipLevelForScale(s->nearzi *
					scale_for_mip *
					pface->texinfo->mipadjust);
			// FIXME: make this passed in to D_CacheSurface
			lmonly = 1; // this is how we know it's lit water that we're drawing
			surfcache_t *pcurrentcache =
				D_CacheSurface(pface, miplevel);
			cacheblock = (u8 *) pcurrentcache->data;
			cachewidth = pcurrentcache->width;
			D_CalcGradients(pface);
			D_DrawSpans8(s->spans); // draw the lightmap to a separate buffer
			miplevel = 0;
			cacheblock = (u8 *)
				((u8 *) pface->texinfo->texture +
				pface->texinfo->texture->offsets[0]);
			cachewidth = 64;
			D_CalcGradients(pface);
			f32 opacity = 1 - (f32)s->entity->alpha / 255;
			if (s->flags & SURF_DRAWLAVA)
				opacity = 1 - r_lavaalpha.value;
			else if (s->flags & SURF_DRAWSLIME)
				opacity = 1 - r_slimealpha.value;
			else if (s->flags & SURF_DRAWWATER)
				opacity = 1 - r_wateralpha.value;
			else if (s->flags & SURF_DRAWTELE)
				opacity = 1 - r_telealpha.value;
			Turbulent8(s->spans, opacity);
			if (!r_wateralphapass) // Manoel Kasimier - translucent water
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
			miplevel = pface->flags & SURF_DRAWCUTOUT ? 0 :
				D_MipLevelForScale(s->nearzi *
					scale_for_mip *
					pface->texinfo->mipadjust);
			// FIXME: make this passed in to D_CacheSurface
			surfcache_t *pcurrentcache =
				D_CacheSurface(pface, miplevel);
			cacheblock = (u8 *) pcurrentcache->data;
			cachewidth = pcurrentcache->width;
			D_CalcGradients(pface);
			D_DrawSpans8(s->spans);
			if (!(pface->flags&SURF_DRAWCUTOUT))
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
