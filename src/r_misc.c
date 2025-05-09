// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

#include "quakedef.h"
#include "r_local.h"

float map_fallbackalpha;
float map_wateralpha;
float map_lavaalpha;
float map_telealpha;
float map_slimealpha;

void R_ParseWorldspawn()
{
	char key[128], value[4096];
	map_fallbackalpha = r_wateralpha.value;
	int ct = cl.worldmodel->contentstransparent;
	map_wateralpha = (ct&SURF_DRAWWATER)?r_wateralpha.value:1;
	map_lavaalpha =  (ct&SURF_DRAWLAVA)? r_lavaalpha.value:1;
	map_telealpha =  (ct&SURF_DRAWTELE)? r_telealpha.value:1;
	map_slimealpha = (ct&SURF_DRAWSLIME)?r_slimealpha.value:1;
	const char *data = COM_Parse(cl.worldmodel->entities);
	if(!data)return; // error
	if(com_token[0] != '{')return; // error
	while(1){
		data = COM_Parse(data);
		if(!data)return; // error
		if(com_token[0]=='}')break; // end of worldspawn
		if(com_token[0]=='_')q_strlcpy(key, com_token + 1, sizeof(key));
		else q_strlcpy(key, com_token, sizeof(key));
		while(key[0]&&key[strlen(key)-1]==' ') // remove trailing spaces
			key[strlen(key)-1] = 0;
		data = COM_ParseEx(data, CPE_ALLOWTRUNC);
		if(!data)return; // error
		q_strlcpy(value, com_token, sizeof(value));
		if(!strcmp("wateralpha", key))map_wateralpha = atof(value);
		if(!strcmp("lavaalpha", key)) map_lavaalpha  = atof(value);
		if(!strcmp("telealpha", key)) map_telealpha  = atof(value);
		if(!strcmp("slimealpha", key))map_slimealpha = atof(value);
	}
}

void R_SetWateralpha_f(cvar_t *var) 
{ // -- ericw
	if (cls.signon == SIGNONS && cl.worldmodel &&
		!(cl.worldmodel->contentstransparent&SURF_DRAWWATER)
		&& var->value < 1)
		Con_Print("Map does not appear to be water-vised\n");
	map_wateralpha = var->value;
	map_fallbackalpha = var->value;
}

void R_SetLavaalpha_f(cvar_t *var)
{
	if (cls.signon == SIGNONS && cl.worldmodel &&
		!(cl.worldmodel->contentstransparent&SURF_DRAWLAVA)
		&& var->value && var->value < 1)
		Con_Print("Map does not appear to be lava-vised\n");
	map_lavaalpha = var->value;
}

void R_SetTelealpha_f(cvar_t *var)
{
	if (cls.signon == SIGNONS && cl.worldmodel &&
		!(cl.worldmodel->contentstransparent&SURF_DRAWTELE)
		&& var->value && var->value < 1)
		Con_Print("Map does not appear to be tele-vised\n");
	map_telealpha = var->value;
}

void R_SetSlimealpha_f(cvar_t *var)
{
	if (cls.signon == SIGNONS && cl.worldmodel &&
		!(cl.worldmodel->contentstransparent&SURF_DRAWSLIME)
		&& var->value && var->value < 1)
		Con_Print("Map does not appear to be slime-vised\n");
	map_slimealpha = var->value;
}

float R_WaterAlphaForTextureType(textype_t type)
{
	if (type == TEXTYPE_LAVA)
		return map_lavaalpha > 0 ? map_lavaalpha : map_fallbackalpha;
	else if (type == TEXTYPE_TELE)
		return map_telealpha > 0 ? map_telealpha : map_fallbackalpha;
	else if (type == TEXTYPE_SLIME)
		return map_slimealpha > 0 ? map_slimealpha : map_fallbackalpha;
	else
		return map_wateralpha;
}

void R_CheckVariables()
{
	static float oldbright;
	if (r_fullbright.value != oldbright) {
		oldbright = r_fullbright.value;
		D_FlushCaches(); // so all lighting changes
	}
}

void R_TimeRefresh_f()
{ // For program optimization
	int startangle = r_refdef.viewangles[1];
	float start = Sys_FloatTime();
	for (int i = 0; i < 128; i++) {
		r_refdef.viewangles[1] = i / 128.0 * 360.0;
		R_RenderView();
		VID_Update();
	}
	float stop = Sys_FloatTime();
	float time = stop - start;
	Con_Printf("%f seconds (%f fps)\n", time, 128 / time);
	r_refdef.viewangles[1] = startangle;
}

void R_LineGraph(int x, int y, int h)
{ // Only called by R_DisplayTime
// FIXME: should be disabled on no-buffer adapters, or should be in the driver
	x += r_refdef.vrect.x;
	y += r_refdef.vrect.y;
	byte *dest = vid.buffer + vid.rowbytes * y + x;
	int s = r_graphheight.value;
	if (h > s)
		h = s;
	int i = 0;
	for (; i < h; i++, dest -= vid.rowbytes * 2) {
		dest[0] = 0xff;
		*(dest - vid.rowbytes) = 0x30;
	}
	for (; i < s; i++, dest -= vid.rowbytes * 2) {
		dest[0] = 0x30;
		*(dest - vid.rowbytes) = 0x30;
	}
}

#define	MAX_TIMINGS 100
void R_TimeGraph()
{ // Performance monitoring tool
	static int timex;
	static byte r_timings[MAX_TIMINGS];
	float r_time2 = Sys_FloatTime();
	int a = (r_time2 - r_time1) / 0.01;
	r_timings[timex] = a;
	a = timex;
	int x;
	if (r_refdef.vrect.width <= MAX_TIMINGS)
		x = r_refdef.vrect.width - 1;
	else
		x = r_refdef.vrect.width -
		    (r_refdef.vrect.width - MAX_TIMINGS) / 2;
	do {
		R_LineGraph(x, r_refdef.vrect.height - 2, r_timings[a]);
		if (x == 0)
			break; // screen too small to hold entire thing
		x--;
		a--;
		if (a == -1)
			a = MAX_TIMINGS - 1;
	} while (a != timex);
	timex = (timex + 1) % MAX_TIMINGS;
}

void R_PrintTimes()
{
	float r_time2 = Sys_FloatTime();
	float ms = 1000 * (r_time2 - r_time1);
	Con_Printf("%5.1f ms %3i/%3i/%3i poly %3i surf\n",
		   ms, c_faceclip, r_polycount, r_drawnpolycount, c_surf);
	c_surf = 0;
}

void R_PrintDSpeeds()
{
	float r_time2 = Sys_FloatTime();
	float dp_time = (dp_time2 - dp_time1) * 1000;
	float rw_time = (rw_time2 - rw_time1) * 1000;
	float db_time = (db_time2 - db_time1) * 1000;
	float se_time = (se_time2 - se_time1) * 1000;
	float de_time = (de_time2 - de_time1) * 1000;
	float dv_time = (dv_time2 - dv_time1) * 1000;
	float ms = (r_time2 - r_time1) * 1000;
	Con_Printf("%3i %4.1fp %3iw %4.1fb %3is %4.1fe %4.1fv\n",
		   (int)ms, dp_time, (int)rw_time, db_time, (int)se_time,
		   de_time, dv_time);
}

void R_PrintAliasStats()
{
	Con_Printf("%3i polygon model drawn\n", r_amodels_drawn);
}

void R_TransformFrustum()
{
	for (int i = 0; i < 4; i++) {
		vec3_t v, v2;
		v[0] = screenedge[i].normal[2];
		v[1] = -screenedge[i].normal[0];
		v[2] = screenedge[i].normal[1];
		v2[0] = v[1] * vright[0] + v[2] * vup[0] + v[0] * vpn[0];
		v2[1] = v[1] * vright[1] + v[2] * vup[1] + v[0] * vpn[1];
		v2[2] = v[1] * vright[2] + v[2] * vup[2] + v[0] * vpn[2];
		VectorCopy(v2, view_clipplanes[i].normal);
		view_clipplanes[i].dist = DotProduct(modelorg, v2);
	}
}

void TransformVector(vec3_t in, vec3_t out)
{
	out[0] = DotProduct(in, vright);
	out[1] = DotProduct(in, vup);
	out[2] = DotProduct(in, vpn);
}

void R_SetUpFrustumIndexes()
{
	int *pindex = r_frustum_indexes;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 3; j++) {
			if (view_clipplanes[i].normal[j] < 0) {
				pindex[j] = j;
				pindex[j + 3] = j + 3;
			} else {
				pindex[j] = j + 3;
				pindex[j + 3] = j;
			}
		}
		pfrustum_indexes[i] = pindex; // FIXME: do just once at start
		pindex += 6;
	}
}

void R_SetupFrame()
{
	if (cl.maxclients > 1) { // don't allow cheats in multiplayer
		Cvar_Set("r_draworder", "0");
		Cvar_Set("r_fullbright", "0");
		Cvar_Set("r_ambient", "0");
		Cvar_Set("r_drawflat", "0");
	}
	if (r_numsurfs.value) {
		if (surface_p - surfaces > r_maxsurfsseen)
			r_maxsurfsseen = surface_p - surfaces;
		Con_Printf("Used %d of %d surfs; %d max\n",
				surface_p - surfaces, surf_max - surfaces,
				r_maxsurfsseen);
	}
	if (r_numedges.value) {
		int edgecount = edge_p - r_edges;

		if (edgecount > r_maxedgesseen)
			r_maxedgesseen = edgecount;

		Con_Printf("Used %d of %d edges; %d max\n", edgecount,
				r_numallocatededges, r_maxedgesseen);
	}
	r_refdef.ambientlight = r_ambient.value;
	if (r_refdef.ambientlight < 0)
		r_refdef.ambientlight = 0;
	if (!sv.active)
		r_draworder.value = 0;	// don't let cheaters look behind walls
	R_CheckVariables();
	R_AnimateLight();
	r_framecount++;
	numbtofpolys = 0;
	// build the transformation matrix for the given view angles
	VectorCopy(r_refdef.vieworg, modelorg);
	VectorCopy(r_refdef.vieworg, r_origin);
	AngleVectors(r_refdef.viewangles, vpn, vright, vup);
	r_oldviewleaf = r_viewleaf; // current viewleaf
	r_viewleaf = Mod_PointInLeaf(r_origin, cl.worldmodel);
	r_dowarpold = r_dowarp;
	r_dowarp = r_waterwarp.value && (r_viewleaf->contents<=CONTENTS_WATER);
	vrect_t vrect;
	if ((r_dowarp != r_dowarpold) || r_viewchanged) {
		if (r_dowarp) {
			if ((vid.width <= vid.maxwarpwidth) &&
					(vid.height <= vid.maxwarpheight)) {
				vrect.x = 0;
				vrect.y = 0;
				vrect.width = vid.width;
				vrect.height = vid.height;
				R_ViewChanged(&vrect, sb_lines, vid.aspect);
			} else {
				float w = vid.width;
				float h = vid.height;
				if (w > vid.maxwarpwidth) {
					h *= (float)vid.maxwarpwidth / w;
					w = vid.maxwarpwidth;
				}
				if (h > vid.maxwarpheight) {
					h = vid.maxwarpheight;
					w *= (float)vid.maxwarpheight / h;
				}
				vrect.x = 0;
				vrect.y = 0;
				vrect.width = (int)w;
				vrect.height = (int)h;
				R_ViewChanged(&vrect, (int)((float)sb_lines*
					(h/(float)vid.height)),vid.aspect*(h/w)*
					((float)vid.width/(float)vid.height));
			}
		} else {
			vrect.x = 0;
			vrect.y = 0;
			vrect.width = vid.width;
			vrect.height = vid.height;
			R_ViewChanged(&vrect, sb_lines, vid.aspect);
		}
		r_viewchanged = false;
	}
	// start off with just the four screen edge clip planes
	R_TransformFrustum();
	VectorCopy(vpn, base_vpn); // save base values
	VectorCopy(vright, base_vright);
	VectorCopy(vup, base_vup);
	VectorCopy(modelorg, base_modelorg);
	R_SetSkyFrame();
	R_SetUpFrustumIndexes();
	r_cache_thrash = false;
	c_faceclip = 0; // clear frame counts
	d_spanpixcount = 0;
	r_polycount = 0;
	r_drawnpolycount = 0;
	r_wholepolycount = 0;
	r_amodels_drawn = 0;
	r_outofsurfaces = 0;
	r_outofedges = 0;
	D_SetupFrame();
}
