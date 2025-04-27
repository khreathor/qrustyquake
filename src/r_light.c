// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

#include "quakedef.h"
#include "r_local.h"

unsigned int r_dlightframecount;
vec3_t lightspot;
vec3_t lightcolor; //johnfitz -- lit support via lordhavoc

void R_AnimateLight() // light animations
{ // 'm' is normal light, 'a' is no light, 'z' is double bright
	int i = (int)(cl.time * 10);
	for (int j = 0; j < MAX_LIGHTSTYLES; j++) {
		if (!cl_lightstyle[j].length) {
			d_lightstylevalue[j] = 256;
			continue;
		}
		int k = i % cl_lightstyle[j].length;
		k = cl_lightstyle[j].map[k] - 'a';
		k = k * 22;
		d_lightstylevalue[j] = k;
	}
}

void R_MarkLights(dlight_t *light, int bit, mnode_t *node)
{
	if (node->contents < 0)
		return;
	mplane_t *splitplane = node->plane;
	float dist = DotProduct(light->origin, splitplane->normal)
		- splitplane->dist;
	if (dist > light->radius) {
		R_MarkLights(light, bit, node->children[0]);
		return;
	}
	if (dist < -light->radius) {
		R_MarkLights(light, bit, node->children[1]);
		return;
	}
	msurface_t *surf = cl.worldmodel->surfaces + node->firstsurface; // mark the polygons
	for (int i = 0; i < node->numsurfaces; i++, surf++) {
		if (surf->dlightframe != r_dlightframecount) {
			surf->dlightbits = 0;
			surf->dlightframe = r_dlightframecount;
		}
		surf->dlightbits |= bit;
	}
	R_MarkLights(light, bit, node->children[0]);
	R_MarkLights(light, bit, node->children[1]);
}

void R_PushDlights()
{
	r_dlightframecount = r_framecount + 1; // because the count hasn't
					       // advanced yet for this frame
	dlight_t *l = cl_dlights;
	for (int i = 0; i < MAX_DLIGHTS; i++, l++) {
		if (l->die < cl.time || !l->radius)
			continue;
		R_MarkLights(l, 1 << i, cl.worldmodel->nodes);
	}
}

/*
int RecursiveLightPoint(mnode_t *node, vec3_t start, vec3_t end)
{
	if (node->contents < 0)
		return -1; // didn't hit anything
	// calculate mid point
	mplane_t *plane = node->plane; // FIXME: optimize for axial
	float front = DotProduct(start, plane->normal) - plane->dist;
	float back = DotProduct(end, plane->normal) - plane->dist;
	int side = front < 0;
	if ((back < 0) == side)
		return RecursiveLightPoint(node->children[side], start, end);
	float frac = front / (front - back);
	vec3_t mid;
	mid[0] = start[0] + (end[0] - start[0]) * frac;
	mid[1] = start[1] + (end[1] - start[1]) * frac;
	mid[2] = start[2] + (end[2] - start[2]) * frac;
	// go down front side   
	int r = RecursiveLightPoint(node->children[side], start, mid);
	if (r >= 0)
		return r; // hit something
	if ((back < 0) == side)
		return -1; // didn't hit anuthing
	// check for impact on this node
	msurface_t *surf = cl.worldmodel->surfaces + node->firstsurface;
	for (int i = 0; i < node->numsurfaces; i++, surf++) {
		if (surf->flags & SURF_DRAWTILED)
			continue; // no lightmaps
		mtexinfo_t *tex = surf->texinfo;
		int s = DotProduct(mid, tex->vecs[0]) + tex->vecs[0][3];
		int t = DotProduct(mid, tex->vecs[1]) + tex->vecs[1][3];;
		if (s < surf->texturemins[0] || t < surf->texturemins[1])
			continue;
		int ds = s - surf->texturemins[0];
		int dt = t - surf->texturemins[1];
		if (ds > surf->extents[0] || dt > surf->extents[1])
			continue;
		if (!surf->samples)
			return 0;
		ds >>= 4;
		dt >>= 4;
		byte *lightmap = surf->samples;
		r = 0;
		if (lightmap) {
			lightmap += dt * ((surf->extents[0] >> 4) + 1) + ds;
			for (int maps = 0; maps < MAXLIGHTMAPS && surf->styles[maps] != 255; maps++) {
				unsigned int scale = d_lightstylevalue[surf->styles[maps]];
				r += *lightmap * scale;
				lightmap += ((surf->extents[0] >> 4) + 1) *
				    ((surf->extents[1] >> 4) + 1);
			}
			r >>= 8;
		}
		return r;
	}
	// go down back side
	return RecursiveLightPoint(node->children[!side], mid, end);
}*/
int RecursiveLightPoint (vec3_t color, mnode_t *node, vec3_t rayorg, vec3_t start, vec3_t end, float *maxdist)
{
	float		front, back, frac;
	vec3_t		mid;

loc0:
	if (node->contents < 0)
		return false;		// didn't hit anything

	// calculate mid point
	if (node->plane->type < 3)
	{
		front = start[node->plane->type] - node->plane->dist;
		back = end[node->plane->type] - node->plane->dist;
	}
	else
	{
		front = DotProduct(start, node->plane->normal) - node->plane->dist;
		back = DotProduct(end, node->plane->normal) - node->plane->dist;
	}

	// LordHavoc: optimized recursion
	if ((back < 0) == (front < 0))
		//		return RecursiveLightPoint (color, node->children[front < 0], rayorg, start, end, maxdist);
	{
		node = node->children[front < 0];
		goto loc0;
	}

	frac = front / (front-back);
	mid[0] = start[0] + (end[0] - start[0])*frac;
	mid[1] = start[1] + (end[1] - start[1])*frac;
	mid[2] = start[2] + (end[2] - start[2])*frac;

	// go down front side
	if (RecursiveLightPoint (color, node->children[front < 0], rayorg, start, mid, maxdist))
		return true;	// hit something
	else
	{
		int i, ds, dt;
		msurface_t *surf;
		// check for impact on this node
		VectorCopy (mid, lightspot);
		//lightplane = node->plane;

		surf = cl.worldmodel->surfaces + node->firstsurface;
		for (i = 0;i < node->numsurfaces;i++, surf++)
		{
			float sfront, sback, dist;
			vec3_t raydelta;

			if (surf->flags & SURF_DRAWTILED)
				continue;	// no lightmaps

			// ericw -- added double casts to force 64-bit precision.
			// Without them the zombie at the start of jam3_ericw.bsp was
			// incorrectly being lit up in SSE builds.
			ds = (int) ((double) DoublePrecisionDotProduct (mid, surf->texinfo->vecs[0]) + surf->texinfo->vecs[0][3]);
			dt = (int) ((double) DoublePrecisionDotProduct (mid, surf->texinfo->vecs[1]) + surf->texinfo->vecs[1][3]);

			if (ds < surf->texturemins[0] || dt < surf->texturemins[1])
				continue;

			ds -= surf->texturemins[0];
			dt -= surf->texturemins[1];

			if (ds > surf->extents[0] || dt > surf->extents[1])
				continue;

			if (surf->plane->type < 3)
			{
				sfront = rayorg[surf->plane->type] - surf->plane->dist;
				sback = end[surf->plane->type] - surf->plane->dist;
			}
			else
			{
				sfront = DotProduct(rayorg, surf->plane->normal) - surf->plane->dist;
				sback = DotProduct(end, surf->plane->normal) - surf->plane->dist;
			}
			VectorSubtract(end, rayorg, raydelta);
			dist = sfront / (sfront - sback) * VectorLength(raydelta);

			if (!surf->samples)
			{
				// We hit a surface that is flagged as lightmapped, but doesn't have actual lightmap info.
				// Instead of just returning black, we'll keep looking for nearby surfaces that do have valid samples.
				// This fixes occasional pitch-black models in otherwise well-lit areas in DOTM (e.g. mge1m1, mge4m1)
				// caused by overlapping surfaces with mixed lighting data.
				const float nearby = 8.f;
				dist += nearby;
				*maxdist = q_min(*maxdist, dist);
				continue;
			}

			if (dist < *maxdist)
			{
				// LordHavoc: enhanced to interpolate lighting
				byte *lightmap;
				int maps, line3, dsfrac = ds & 15, dtfrac = dt & 15, r00 = 0, g00 = 0, b00 = 0, r01 = 0, g01 = 0, b01 = 0, r10 = 0, g10 = 0, b10 = 0, r11 = 0, g11 = 0, b11 = 0;
				float scale;
				line3 = ((surf->extents[0]>>4)+1)*3;

				lightmap = surf->samples + ((dt>>4) * ((surf->extents[0]>>4)+1) + (ds>>4))*3; // LordHavoc: *3 for color

				for (maps = 0;maps < MAXLIGHTMAPS && surf->styles[maps] != 255;maps++)
				{
					scale = (float) d_lightstylevalue[surf->styles[maps]] * 1.0 / 256.0;
					r00 += (float) lightmap[      0] * scale;g00 += (float) lightmap[      1] * scale;b00 += (float) lightmap[2] * scale;
					r01 += (float) lightmap[      3] * scale;g01 += (float) lightmap[      4] * scale;b01 += (float) lightmap[5] * scale;
					r10 += (float) lightmap[line3+0] * scale;g10 += (float) lightmap[line3+1] * scale;b10 += (float) lightmap[line3+2] * scale;
					r11 += (float) lightmap[line3+3] * scale;g11 += (float) lightmap[line3+4] * scale;b11 += (float) lightmap[line3+5] * scale;
					lightmap += ((surf->extents[0]>>4)+1) * ((surf->extents[1]>>4)+1)*3; // LordHavoc: *3 for colored lighting
				}

				color[0] += (float) ((int) ((((((((r11-r10) * dsfrac) >> 4) + r10)-((((r01-r00) * dsfrac) >> 4) + r00)) * dtfrac) >> 4) + ((((r01-r00) * dsfrac) >> 4) + r00)));
				color[1] += (float) ((int) ((((((((g11-g10) * dsfrac) >> 4) + g10)-((((g01-g00) * dsfrac) >> 4) + g00)) * dtfrac) >> 4) + ((((g01-g00) * dsfrac) >> 4) + g00)));
				color[2] += (float) ((int) ((((((((b11-b10) * dsfrac) >> 4) + b10)-((((b01-b00) * dsfrac) >> 4) + b00)) * dtfrac) >> 4) + ((((b01-b00) * dsfrac) >> 4) + b00)));
			}
			return true; // success
		}

		// go down back side
		return RecursiveLightPoint (color, node->children[front >= 0], rayorg, mid, end, maxdist);
	}
}

int R_LightPoint(vec3_t p)
{
	if (!cl.worldmodel->lightdata) {
		lightcolor[0] = lightcolor[1] = lightcolor[2] = 255;
		return 255;
	}
	float maxdist = 8192; //johnfitz -- was 2048
	vec3_t end;
	end[0] = p[0];
	end[1] = p[1];
	end[2] = p[2] - 8192;
	lightcolor[0] = lightcolor[1] = lightcolor[2] = 0;
	RecursiveLightPoint(lightcolor,cl.worldmodel->nodes,p,p,end,&maxdist);
	if (r_rgblighting.value && !lit_lut_initialized &&
		!(lightcolor[0]==lightcolor[1] && lightcolor[0]==lightcolor[2]))
		R_BuildLitLUT();
	return ((lightcolor[0] + lightcolor[1] + lightcolor[2]) * (1.0f/3.0f));
}
