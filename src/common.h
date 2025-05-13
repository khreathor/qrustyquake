// Copyright (C) 1996-2001 Id Software, Inc.
// Copyright (C) 2002-2009 John Fitzgibbons and others
// Copyright (C) 2010-2014 QuakeSpasm developers
// GPLv3 See LICENSE for details.

#ifndef _Q_COMMON_H
#define _Q_COMMON_H

// comndef.h  -- general definitions

void SZ_Alloc (sizebuf_t *buf, s32 startsize);
void SZ_Free (sizebuf_t *buf);
void SZ_Clear (sizebuf_t *buf);
void *SZ_GetSpace (sizebuf_t *buf, s32 length);
void SZ_Write (sizebuf_t *buf, const void *data, s32 length);
void SZ_Print (sizebuf_t *buf, const s8 *data);	// strcats onto the sizebuf

//============================================================================



void ClearLink (link_t *l);
void RemoveLink (link_t *l);
void InsertLinkBefore (link_t *l, link_t *before);
void InsertLinkAfter (link_t *l, link_t *after);

// (type *)STRUCT_FROM_LINK(link_t *link, type, member)
// ent = STRUCT_FROM_LINK(link,entity_t,order)
// FIXME: remove this mess!

//============================================================================



void Vec_Grow (void **pvec, size_t element_size, size_t count);
void Vec_Append (void **pvec, size_t element_size, const void *data, size_t count);
void Vec_Clear (void **pvec);
void Vec_Free (void **pvec);

//============================================================================

extern	bool		host_bigendian;

extern	s16	(*BigShort) (s16 l);
extern	s16	(*LittleShort) (s16 l);
extern	s32	(*BigLong) (s32 l);
extern	s32	(*LittleLong) (s32 l);
extern	float	(*BigFloat) (float l);
extern	float	(*LittleFloat) (float l);

//============================================================================

void MSG_WriteChar (sizebuf_t *sb, s32 c);
void MSG_WriteByte (sizebuf_t *sb, s32 c);
void MSG_WriteShort (sizebuf_t *sb, s32 c);
void MSG_WriteLong (sizebuf_t *sb, s32 c);
void MSG_WriteFloat (sizebuf_t *sb, float f);
void MSG_WriteString (sizebuf_t *sb, const s8 *s);
void MSG_WriteCoord (sizebuf_t *sb, float f, u32 flags);
void MSG_WriteAngle (sizebuf_t *sb, float f, u32 flags);
void MSG_WriteAngle16 (sizebuf_t *sb, float f, u32 flags); //johnfitz

extern	s32			msg_readcount;
extern	bool	msg_badread;		// set if a read goes beyond end of message

void MSG_BeginReading (void);
s32 MSG_ReadChar (void);
s32 MSG_ReadByte (void);
s32 MSG_ReadShort (void);
s32 MSG_ReadLong (void);
float MSG_ReadFloat (void);
const s8 *MSG_ReadString (void);

float MSG_ReadCoord (u32 flags);
float MSG_ReadAngle (u32 flags);
float MSG_ReadAngle16 (u32 flags); //johnfitz

//============================================================================

void Q_memset (void *dest, s32 fill, size_t count);
void Q_memmove (void *dest, const void *src, size_t count);
void Q_memcpy (void *dest, const void *src, size_t count);
s32 Q_memcmp (const void *m1, const void *m2, size_t count);
void Q_strcpy (s8 *dest, const s8 *src);
void Q_strncpy (s8 *dest, const s8 *src, s32 count);
s32 Q_strlen (const s8 *str);
s8 *Q_strrchr (const s8 *s, s8 c);
void Q_strcat (s8 *dest, const s8 *src);
s32 Q_strcmp (const s8 *s1, const s8 *s2);
s32 Q_strncmp (const s8 *s1, const s8 *s2, s32 count);
s32	Q_atoi (const s8 *str);
float Q_atof (const s8 *str);


/* locale-insensitive strcasecmp replacement functions: */
extern s32 q_strcasecmp (const s8 * s1, const s8 * s2);
extern s32 q_strncasecmp (const s8 *s1, const s8 *s2, size_t n);

/* locale-insensitive case-insensitive alternative to strstr */
extern s8 *q_strcasestr(const s8 *haystack, const s8 *needle);

/* locale-insensitive strlwr/upr replacement functions: */
extern s8 *q_strlwr (s8 *str);
extern s8 *q_strupr (s8 *str);

// strdup that calls malloc
extern s8 *q_strdup (const s8 *str);

/* snprintf, vsnprintf : always use our versions. */
extern s32 q_snprintf (s8 *str, size_t size, const s8 *format, ...);// FUNC_PRINTF(3,4);
extern s32 q_vsnprintf(s8 *str, size_t size, const s8 *format, va_list args);// FUNC_PRINTF(3,0);

//============================================================================

extern	s8		com_token[1024];
extern	bool	com_eof;


const s8 *COM_Parse (const s8 *data);
const s8 *COM_ParseEx (const s8 *data, cpe_mode mode);


extern	s32		com_argc;
extern	s8	**com_argv;

extern	s32		safemode;
/* safe mode: in true, the engine will behave as if one
   of these arguments were actually on the command line:
   -nosound, -nocdaudio, -nomidi, -stdvid, -dibonly,
   -nomouse, -nojoy, -nolan
 */

s32 COM_CheckParm (const s8 *parm);

void COM_Init (void);
void COM_InitArgv (s32 argc, s8 **argv);
void COM_InitFilesystem (void);

const s8 *COM_SkipPath (const s8 *pathname);
void COM_StripExtension (const s8 *in, s8 *out, size_t outsize);
void COM_FileBase (const s8 *in, s8 *out, size_t outsize);
void COM_AddExtension (s8 *path, const s8 *extension, size_t len);
#if 0 /* COM_DefaultExtension can be dangerous */
void COM_DefaultExtension (s8 *path, const s8 *extension, size_t len);
#endif
const s8 *COM_FileGetExtension (const s8 *in); /* doesn't return NULL */
void COM_ExtractExtension (const s8 *in, s8 *out, size_t outsize);
void COM_CreatePath (s8 *path);

s8 *va (const s8 *format, ...);// FUNC_PRINTF(1,2);
// does a varargs printf into a temp buffer

unsigned COM_HashString (const s8 *str);

// localization support for 2021 rerelease version:
void LOC_Init (void);
void LOC_Shutdown (void);
const s8* LOC_GetRawString (const s8 *key);
const s8* LOC_GetString (const s8 *key);
bool LOC_HasPlaceholders (const s8 *str);
size_t LOC_Format (const s8 *format, const s8* (*getarg_fn)(s32 idx, void* userdata), void* userdata, s8* out, size_t len);

//============================================================================

// QUAKEFS

extern searchpath_t *com_searchpaths;
extern searchpath_t *com_base_searchpaths;

extern s32 com_filesize;
struct cache_user_s;

extern	s8	com_basedir[MAX_OSPATH];
extern	s8	com_gamedir[MAX_OSPATH];
extern	s32	file_from_pak;	// global indicating that file came from a pak

void COM_WriteFile (const s8 *filename, const void *data, s32 len);
s32 COM_OpenFile (const s8 *filename, s32 *handle, u32 *path_id);
s32 COM_FOpenFile (const s8 *filename, FILE **file, u32 *path_id);
bool COM_FileExists (const s8 *filename, u32 *path_id);
void COM_CloseFile (s32 h);

// these procedures open a file using COM_FindFile and loads it into a proper
// buffer. the buffer is allocated with a total size of com_filesize + 1. the
// procedures differ by their buffer allocation method.
byte *COM_LoadStackFile (const s8 *path, void *buffer, s32 bufsize,
						u32 *path_id);
	// uses the specified stack stack buffer with the specified size
	// of bufsize. if bufsize is too s16, uses temp hunk. the bufsize
	// must include the +1
byte *COM_LoadTempFile (const s8 *path, u32 *path_id);
	// allocates the buffer on the temp hunk.
byte *COM_LoadHunkFile (const s8 *path, u32 *path_id);
	// allocates the buffer on the hunk.
byte *COM_LoadZoneFile (const s8 *path, u32 *path_id);
	// allocates the buffer on the zone.
void COM_LoadCacheFile (const s8 *path, struct cache_user_s *cu,
						u32 *path_id);
	// uses cache mem for allocating the buffer.
byte *COM_LoadMallocFile (const s8 *path, u32 *path_id);
	// allocates the buffer on the system mem (malloc).

// Opens the given path directly, ignoring search paths.
// Returns NULL on failure, or else a '\0'-terminated malloc'ed buffer.
// Loads in "t" mode so CRLF to LF translation is performed on Windows.
byte *COM_LoadMallocFile_TextMode_OSPath (const s8 *path, s64 *len_out);

// Attempts to parse an s32, followed by a newline.
// Returns advanced buffer position.
// Doesn't signal parsing failure, but this is not needed for savegame loading.
const s8 *COM_ParseIntNewline(const s8 *buffer, s32 *value);

// Attempts to parse a float followed by a newline.
// Returns advanced buffer position.
const s8 *COM_ParseFloatNewline(const s8 *buffer, float *value);

// Parse a string of non-whitespace into com_token, then tries to consume a
// newline. Returns advanced buffer position.
const s8 *COM_ParseStringNewline(const s8 *buffer);



/* The following FS_*() stdio replacements are necessary if one is
 * to perform non-sequential reads on files reopened on pak files
 * because we need the bookkeeping about file start/end positions.
 * Allocating and filling in the fshandle_t structure is the users'
 * responsibility when the file is initially opened. */


size_t FS_fread(void *ptr, size_t size, size_t nmemb, fshandle_t *fh);
s32 FS_fseek(fshandle_t *fh, s64 offset, s32 whence);
s64 FS_ftell(fshandle_t *fh);
void FS_rewind(fshandle_t *fh);
s32 FS_feof(fshandle_t *fh);
s32 FS_ferror(fshandle_t *fh);
s32 FS_fclose(fshandle_t *fh);
s32 FS_fgetc(fshandle_t *fh);
s8 *FS_fgets(s8 *s, s32 size, fshandle_t *fh);
s64 FS_filelength (fshandle_t *fh);
s32 q_strlcpy (s8 *dst, const s8 *src, size_t siz);
size_t q_strlcat (s8 *dst, const s8 *src, size_t siz);

extern struct cvar_s	registered;
extern bool		standard_quake, rogue, hipnotic;
extern bool		fitzmode;
	/* if true, run in fitzquake mode disabling custom quakespasm hacks */

#endif	/* _Q_COMMON_H */
