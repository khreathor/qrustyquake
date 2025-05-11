#ifndef GLOBDEFINES_
#define GLOBDEFINES_

#define TEX_SPECIAL 1 // sky or slime, no lightmap or 256 subdivision // bspfile
#define TEX_MISSING 2 // johnfitz -- this texinfo does not have a texture
#define MAX_MAP_HULLS 4 // upper design bounds
#define MAX_MAP_MODELS 256
#define MAX_MAP_BRUSHES 4096
#define MAX_MAP_ENTITIES 1024
#define MAX_MAP_ENTSTRING 65536
#define MAX_MAP_PLANES 32767
#define MAX_MAP_NODES 32767 // because negative shorts are contents
#define MAX_MAP_CLIPNODES 32767
#define MAX_MAP_LEAFS 32767
#define MAX_MAP_VERTS 65535
#define MAX_MAP_FACES 65535
#define MAX_MAP_MARKSURFACES 65535
#define MAX_MAP_TEXINFO 4096
#define MAX_MAP_EDGES 256000
#define MAX_MAP_SURFEDGES 512000
#define MAX_MAP_TEXTURES 512
#define MAX_MAP_MIPTEX 0x200000
#define MAX_MAP_LIGHTING 0x100000
#define MAX_MAP_VISIBILITY 0x100000
#define MAX_MAP_PORTALS 65536
#define MAX_KEY 32 // key / value pair sizes
#define MAX_VALUE 1024
#define BSPVERSION 29
#ifdef BSP29_VALVE
#define BSPVERSION_VALVE 30
#endif
// RMQ support (2PSB). 32bits instead of shorts for all but bbox sizes (which
// still use shorts)
#define BSP2VERSION_2PSB (('B' << 24) | ('S' << 16) | ('P' << 8) | '2')
// BSP2 support. 32bits instead of shorts for everything (bboxes use floats)
#define BSP2VERSION_BSP2 (('B' << 0) | ('S' << 8) | ('P' << 16) | ('2'<<24))
// Quake64
#define BSPVERSION_QUAKE64 (('Q' << 24) | ('6' << 16) | ('4' << 8) | ' ')
#define TOOLVERSION     2
#define LUMP_ENTITIES 0
#define LUMP_PLANES 1
#define LUMP_TEXTURES 2
#define LUMP_VERTEXES 3
#define LUMP_VISIBILITY 4
#define LUMP_NODES 5
#define LUMP_TEXINFO 6
#define LUMP_FACES 7
#define LUMP_LIGHTING 8
#define LUMP_CLIPNODES 9
#define LUMP_LEAFS 10
#define LUMP_MARKSURFACES 11
#define LUMP_EDGES 12
#define LUMP_SURFEDGES 13
#define LUMP_MODELS 14
#define HEADER_LUMPS 15
#define MIPLEVELS 4
#define PLANE_X 0 // 0-2 are axial planes
#define PLANE_Y 1
#define PLANE_Z 2
#define PLANE_ANYX 3 // 3-5 are non-axial planes snapped to the nearest
#define PLANE_ANYY 4
#define PLANE_ANYZ 5
#define CONTENTS_EMPTY -1
#define CONTENTS_SOLID -2
#define CONTENTS_WATER -3
#define CONTENTS_SLIME -4
#define CONTENTS_LAVA -5
#define CONTENTS_SKY -6
#define CONTENTS_ORIGIN -7 // removed at csg time
#define CONTENTS_CLIP -8 // changed to contents_solid
#define CONTENTS_CURRENT_0 -9
#define CONTENTS_CURRENT_90 -10
#define CONTENTS_CURRENT_180 -11
#define CONTENTS_CURRENT_270 -12
#define CONTENTS_CURRENT_UP -13
#define CONTENTS_CURRENT_DOWN -14
#define TEX_SPECIAL 1 // sky or slime, no lightmap or 256 subdivision
#define MAXLIGHTMAPS 4
#define AMBIENT_WATER 0
#define AMBIENT_SKY 1
#define AMBIENT_SLIME 2
#define AMBIENT_LAVA 3
#define NUM_AMBIENTS 4 // automatic ambient sounds

#define SPRITE_VERSION 1                                             // spritegn
#define SPR_VP_PARALLEL_UPRIGHT 0
#define SPR_FACING_UPRIGHT 1
#define SPR_VP_PARALLEL 2
#define SPR_ORIENTED 3
#define SPR_VP_PARALLEL_ORIENTED 4
#define IDSPRITEHEADER (('P'<<24)+('S'<<16)+('D'<<8)+'I') // little-endian IDSP

#define ALIAS_VERSION 6                                              // modelgen
#define ALIAS_ONSEAM 0x0020
#define DT_FACES_FRONT 0x0010
#define IDPOLYHEADER (('O'<<24)+('P'<<16)+('D'<<8)+'I') // little-endian "IDPO"

#define SIDE_FRONT 0                                                    // model
#define SIDE_BACK 1
#define SIDE_ON 2
#define SURF_PLANEBACK 2
#define SURF_DRAWSKY 4
#define SURF_DRAWSPRITE 8
#define SURF_DRAWTURB 0x10
#define SURF_DRAWTILED 0x20
#define SURF_DRAWBACKGROUND 0x40
#define SURF_DRAWCUTOUT 0x80
#define SURF_DRAWLAVA 0x400
#define SURF_DRAWSLIME 0x800
#define SURF_DRAWTELE 0x1000
#define SURF_DRAWWATER 0x2000
#define SURF_DRAWSKYBOX 0x80000 // Manoel Kasimier - skyboxes
#define SURF_WINQUAKE_DRAWTRANSLUCENT (SURF_DRAWLAVA | SURF_DRAWSLIME | SURF_DRAWWATER | SURF_DRAWTELE)
#define EF_ROCKET 1 // leave a trail
#define EF_GRENADE 2 // leave a trail
#define EF_GIB 4 // leave a trail
#define EF_ROTATE 8 // rotate(bonus items)
#define EF_TRACER 16 // green split trail
#define EF_ZOMGIB 32 // small blood trail
#define EF_TRACER2 64 // orange split trail + rotate
#define EF_TRACER3 128 // purple trail
#define EF_BRIGHTFIELD 1 // entity effects
#define EF_MUZZLEFLASH 2
#define EF_BRIGHTLIGHT 4
#define EF_DIMLIGHT 8
#define EF_QEX_QUADLIGHT 16 // 2021 rerelease
#define EF_QEX_PENTALIGHT 32 // 2021 rerelease
#define EF_QEX_CANDLELIGHT 64 // 2021 rerelease
#define SIDE_FRONT 0
#define SIDE_BACK 1
#define SIDE_ON 2
#define SURF_PLANEBACK 2
#define SURF_DRAWSKY 4
#define SURF_DRAWSPRITE 8
#define SURF_DRAWTURB 0x10
#define SURF_DRAWTILED 0x20
#define SURF_DRAWBACKGROUND 0x40
#define SURF_UNDERWATER 0x80
#define SURF_NOTEXTURE 0x100 //johnfitz
#define SURF_DRAWFENCE 0x200
#define SURF_DRAWLAVA 0x400
#define SURF_DRAWSLIME 0x800
#define SURF_DRAWTELE 0x1000
#define SURF_DRAWWATER 0x2000
#define VERTEXSIZE 7
#define MAX_SKINS 32
#define MAXALIASVERTS 3984 //Baker: 1024 is GLQuake original limit. 2000 is WinQuake original limit. Baker 2000->3984
#define MAXALIASFRAMES 1024 //spike -- was 256
#define MAXALIASTRIS 4096 //ericw -- was 2048
#define EF_ROCKET 1 // leave a trail
#define EF_GRENADE 2 // leave a trail
#define EF_GIB 4 // leave a trail
#define EF_ROTATE 8 // rotate (bonus items)
#define EF_TRACER 16 // green split trail
#define EF_ZOMGIB 32 // small blood trail
#define EF_TRACER2 64 // orange split trail + rotate
#define EF_TRACER3 128 // purple trail
#define MF_HOLEY (1u<<14) // MarkV/QSS -- make index 255 transparent on mdl's
#define MOD_NOSHADOW 512 //don't cast a shadow
#define MOD_FBRIGHTHACK 1024 //when fullbrights are disabled, use a hack to render this model brighter
#define TEXPREF_NONE 0x0000
#define TEXPREF_MIPMAP 0x0001 // generate mipmaps
			      // TEXPREF_NEAREST and TEXPREF_LINEAR aren't supposed to be ORed with TEX_MIPMAP
#define TEXPREF_LINEAR 0x0002 // force linear
#define TEXPREF_NEAREST 0x0004 // force nearest
#define TEXPREF_ALPHA 0x0008 // allow alpha
#define TEXPREF_PAD 0x0010 // allow padding
#define TEXPREF_PERSIST 0x0020 // never free
#define TEXPREF_OVERWRITE 0x0040 // overwrite existing same-name texture
#define TEXPREF_NOPICMIP 0x0080 // always load full-sized
#define TEXPREF_FULLBRIGHT 0x0100 // use fullbright mask palette
#define TEXPREF_NOBRIGHT 0x0200 // use nobright mask palette
#define TEXPREF_CONCHARS 0x0400 // use conchars palette
#define TEXPREF_WARPIMAGE 0x0800 // resize this texture when warpimagesize changes
//johnfitz -- extra flags for rendering
#define MOD_NOLERP 256 // don't lerp when animating
#define MOD_NOSHADOW 512 // don't cast a shadow
#define MOD_FBRIGHTHACK 1024 // when fullbrights are disabled, use a hack to render this model brighter
#endif
