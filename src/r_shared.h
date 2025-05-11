// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

// r_shared.h: general refresh-related stuff shared between the refresh and the
// driver

// FIXME: clean up and move into d_iface.h

#ifndef _R_SHARED_H_
#define _R_SHARED_H_
extern int cachewidth;
extern pixel_t *cacheblock;
extern int screenwidth;
extern float pixelAspect;
extern int r_drawnpolycount;
extern int sintable[SIN_BUFFER_SIZE];
extern int intsintable[SIN_BUFFER_SIZE];
extern vec3_t vup, base_vup;
extern vec3_t vpn, base_vpn;
extern vec3_t vright, base_vright;
extern entity_t *currententity;
extern vec3_t sxformaxis[4]; // s axis transformed into viewspace
extern vec3_t txformaxis[4]; // t axis transformed into viewspac
extern vec3_t modelorg, base_modelorg;
extern float xcenter, ycenter;
extern float xscale, yscale;
extern float xscaleinv, yscaleinv;
extern float xscaleshrink, yscaleshrink;
extern int d_lightstylevalue[256]; // 8.8 frac of base light value
extern int r_skymade;
extern char skybox_name[1024];
extern int ubasestep, errorterm, erroradjustup, erroradjustdown;
// surfaces are generated in back to front order by the bsp, so if a surf
// pointer is greater than another one, it should be drawn in front
// surfaces[1] is the background, and is used as the active surface stack.
// surfaces[0] is a dummy, because index 0 is used to indicate no surface
// attached to an edge_t
extern surf_t *surfaces, *surface_p, *surf_max;

extern void TransformVector(vec3_t in, vec3_t out);
extern void SetUpForLineScan(fixed8_t startvertu, fixed8_t startvertv,
		fixed8_t endvertu, fixed8_t endvertv);
extern void R_MakeSky();
extern void R_DrawLine(polyvert_t *polyvert0, polyvert_t *polyvert1);
#endif
