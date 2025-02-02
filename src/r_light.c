// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

#include "quakedef.h"
#include "r_local.h"

unsigned int r_dlightframecount;

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
			for (int maps = 0;
			     maps < MAXLIGHTMAPS && surf->styles[maps] != 255;
			     maps++) {
				unsigned int scale =
					d_lightstylevalue[surf->styles[maps]];
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
}

int R_LightPoint(vec3_t p)
{
	if (!cl.worldmodel->lightdata)
		return 255;
	vec3_t end;
	end[0] = p[0];
	end[1] = p[1];
	end[2] = p[2] - 2048;
	int r = RecursiveLightPoint(cl.worldmodel->nodes, p, end);
	if (r == -1)
		r = 0;
	if (r < r_refdef.ambientlight)
		r = r_refdef.ambientlight;
	return r;
}
