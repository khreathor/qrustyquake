// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

// spritegn.h: header file for sprite generation program

// **********************************************************
// * This file must be identical in the spritegen directory *
// * and in the Quake directory, because it's used to    *
// * pass data from one to the other via .spr files.     *
// **********************************************************

//-------------------------------------------------------
// This program generates .spr sprite package files.
// The format of the files is as follows:
//
// dsprite_t file header structure
// <repeat dsprite_t.numframes times>
// <if spritegroup, repeat dspritegroup_t.numframes times>
//  dspriteframe_t frame header structure
//  sprite bitmap
// <else (single sprite frame)>
//  dspriteframe_t frame header structure
//  sprite bitmap
// <endrepeat>
//-------------------------------------------------------

#define SPRITE_VERSION 1
#define SPR_VP_PARALLEL_UPRIGHT 0
#define SPR_FACING_UPRIGHT 1
#define SPR_VP_PARALLEL 2
#define SPR_ORIENTED 3
#define SPR_VP_PARALLEL_ORIENTED 4
#define IDSPRITEHEADER (('P'<<24)+('S'<<16)+('D'<<8)+'I') // little-endian IDSP
#ifndef SYNCTYPE_T
#define SYNCTYPE_T // must match definition in modelgen.h
typedef enum { ST_SYNC=0, ST_RAND } synctype_t;
#endif

typedef enum { SPR_SINGLE=0, SPR_GROUP } spriteframetype_t;

typedef struct { // TODO: shorten these?
	int ident;
	int version;
	int type;
	float boundingradius;
	int width;
	int height;
	int numframes;
	float beamlength;
	synctype_t synctype;
} dsprite_t;

typedef struct {
	int origin[2];
	int width;
	int height;
} dspriteframe_t;

typedef struct {
	int numframes;
} dspritegroup_t;

typedef struct {
	float interval;
} dspriteinterval_t;

typedef struct {
	spriteframetype_t type;
} dspriteframetype_t;
