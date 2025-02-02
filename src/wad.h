// Copyright (C) 1996-2001 Id Software, Inc.
// Copyright (C) 2002-2009 John Fitzgibbons and others
// Copyright (C) 2010-2014 QuakeSpasm developers
// GPLv3 See LICENSE for details.

#ifndef _QUAKE_WAD_H
#define _QUAKE_WAD_H

//===============
//   TYPES
//===============

#define	CMP_NONE		0
#define	CMP_LZSS		1

#define	TYP_NONE		0
#define	TYP_LABEL		1

#define	TYP_LUMPY		64				// 64 + grab command number
#define	TYP_PALETTE		64
#define	TYP_QTEX		65
#define	TYP_QPIC		66
#define	TYP_SOUND		67
#define	TYP_MIPTEX_PALETTE	67
#define	TYP_MIPTEX		68

#define WADID		('W' | ('A' << 8) | ('D' << 16) | ('2' << 24))
#define WADID_VALVE	('W' | ('A' << 8) | ('D' << 16) | ('3' << 24))

#define	WADFILENAME "gfx.wad" //johnfitz -- filename is now hard-coded for honesty

typedef struct
{
	int			width, height;
	byte		data[4];			// variably sized
} qpic_t;

typedef struct
{
	char		identification[4];		// should be WAD2 or 2DAW
	int			numlumps;
	int			infotableofs;
} wadinfo_t;

typedef struct
{
	int			filepos;
	int			disksize;
	int			size;					// uncompressed
	char		type;
	char		compression;
	char		pad1, pad2;
	char		name[16];				// must be null terminated
} lumpinfo_t;

typedef struct wad_s
{
	char		name[MAX_QPATH];
	int			id;
	fshandle_t	fh;
	int			numlumps;
	lumpinfo_t	*lumps;
	struct wad_s	*next;
} wad_t;

extern	int			wad_numlumps;
extern	lumpinfo_t	*wad_lumps;
extern	byte		*wad_base;

void	W_LoadWadFile (void); //johnfitz -- filename is now hard-coded for honesty
void	W_CleanupName (const char *in, char *out);
void	*W_GetLumpName (const char *name);
void	*W_GetLumpNum (int num);

wad_t	*W_LoadWadList (const char *names);
void	W_FreeWadList (wad_t *wads);
lumpinfo_t *W_GetLumpinfoList (wad_t *wads, const char *name, wad_t **out_wad);

void SwapPic (qpic_t *pic);

#endif	/* _QUAKE_WAD_H */

