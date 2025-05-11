// Copyright(C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

// refresh.h -- public interface to refresh functions


// refresh
extern int reinit_surfcache;
extern vec3_t r_origin, vpn, vright, vup;
// surface cache related
extern int reinit_surfcache; // if 1, surface cache is currently empty and
extern qboolean r_cache_thrash; // set if thrashing the surface cache

void R_Init();
void R_InitTextures();
void R_InitEfrags();
void R_RenderView(); // must set r_refdef first
void R_ViewChanged(vrect_t *pvrect, int lineadj, float aspect); // called
	// whenever r_refdef or vid change
void R_InitSky(struct texture_s *mt); // called at level load
void R_AddEfrags(entity_t *ent);
void R_RemoveEfrags(entity_t *ent);
void R_NewMap();
void R_ParseParticleEffect();
void R_RunParticleEffect(vec3_t org, vec3_t dir, int color, int count);
void R_RocketTrail(vec3_t start, vec3_t end, int type);
void R_EntityParticles(entity_t *ent);
void R_BlobExplosion(vec3_t org);
void R_ParticleExplosion(vec3_t org);
void R_ParticleExplosion2(vec3_t org, int colorStart, int colorLength);
void R_LavaSplash(vec3_t org);
void R_TeleportSplash(vec3_t org);
void R_PushDlights();
// surface cache related
int D_SurfaceCacheForRes(int width, int height);
void D_FlushCaches();
void D_DeleteSurfaceCache();
void D_InitCaches(void *buffer, int size);
void R_SetVrect(vrect_t *pvrect, vrect_t *pvrectin, int lineadj);
