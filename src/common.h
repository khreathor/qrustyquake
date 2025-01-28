// Copyright (C) 1996-2001 Id Software, Inc.
// Copyright (C) 2002-2009 John Fitzgibbons and others
// GPLv3 See LICENSE for details.

#define LOADFILE_ZONE           0
#define LOADFILE_HUNK           1
#define LOADFILE_TEMPHUNK       2
#define LOADFILE_CACHE          3
#define LOADFILE_STACK          4
#define LOADFILE_MALLOC         5
#define MAX_FILES_IN_PACK 2048
#define Q_MAXCHAR ((char)0x7f)
#define Q_MAXSHORT ((short)0x7fff)
#define Q_MAXINT ((int)0x7fffffff)
#define Q_MAXLONG ((int)0x7fffffff)
#define Q_MAXFLOAT ((int)0x7fffffff)
#define Q_MINCHAR ((char)0x80)
#define Q_MINSHORT ((short)0x8000)
#define Q_MININT ((int)0x80000000)
#define Q_MINLONG ((int)0x80000000)
#define Q_MINFLOAT ((int)0x7fffffff)
#if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
#define host_bigendian 1
#else
#define host_bigendian 0
#endif
#define BigShort(s) ((short)SDL_SwapBE16((s)))
#define LittleShort(s) ((short)SDL_SwapLE16((s)))
#define BigLong(l) ((int)SDL_SwapBE32((l)))
#define LittleLong(l) ((int)SDL_SwapLE32((l)))
#define BigFloat(f) SDL_SwapFloatBE((f))
#define LittleFloat(f) SDL_SwapFloatLE((f))
#define STRUCT_FROM_LINK(l,t,m) ((t *)((byte *)l - offsetof(t,m)))

#define VEC_HEADER(v) (((vec_header_t*)(v))[-1])
#define VEC_PUSH(v,n) do { Vec_Grow((void**)&(v), sizeof((v)[0]), 1); (v)[VEC_HEADER(v).size++] = (n); } while (0)
#define VEC_SIZE(v)   ((v) ? VEC_HEADER(v).size : 0)
#define VEC_FREE(v)   Vec_Free((void**)&(v))
#define VEC_CLEAR(v)  Vec_Clear((void**)&(v))

typedef enum
{
        CPE_NOTRUNC,            // return parse error in case of overflow
        CPE_ALLOWTRUNC          // truncate com_token in case of overflow
} cpe_mode;

typedef unsigned char byte;
typedef int qboolean;

typedef struct sizebuf_s
{
	qboolean allowoverflow; // if false, do a Sys_Error
	qboolean overflowed; // set to true if the buffer size failed
	byte *data;
	int maxsize;
	int cursize;
} sizebuf_t;

typedef struct { // in memory
	char name[MAX_QPATH];
	int filepos, filelen;
} packfile_t;

typedef struct pack_s {
	char filename[MAX_OSPATH];
	int handle;
	int numfiles;
	packfile_t *files;
} pack_t;

typedef struct { // on disk
	char name[56];
	int filepos, filelen;
} dpackfile_t;

typedef struct {
	char id[4];
	int dirofs;
	int dirlen;
} dpackheader_t;

typedef struct searchpath_s {
	char filename[MAX_OSPATH];
	pack_t *pack; // only one of filename / pack will be used
	struct searchpath_s *next;
	int path_id;
} searchpath_t;

typedef struct _fshandle_t
{
        FILE *file;
        qboolean pak;   /* is the file read from a pak */
        long start;     /* file or data start position */
        long length;    /* file or data size */
        long pos;       /* current position relative to start */
} fshandle_t;

typedef struct link_s
{
	struct link_s *prev, *next;
} link_t;

extern qboolean bigendien;
extern int msg_readcount;
extern qboolean msg_badread; // set if a read goes beyond end of message
extern char com_token[1024];
extern qboolean com_eof;
extern int com_argc;
extern char **com_argv;
extern int com_filesize;
struct cache_user_s;
extern char com_gamedir[MAX_OSPATH];
extern struct cvar_s registered;
extern qboolean standard_quake, rogue, hipnotic;

void ClearLink(link_t *l);
void RemoveLink(link_t *l);
void InsertLinkBefore(link_t *l, link_t *before);
extern short(*BigShort)(short l);
extern short(*LittleShort)(short l);
extern int(*BigLong)(int l);
extern int(*LittleLong)(int l);
extern float(*BigFloat)(float l);
extern float(*LittleFloat)(float l);
void MSG_WriteChar(sizebuf_t *sb, int c);
void MSG_WriteByte(sizebuf_t *sb, int c);
void MSG_WriteShort(sizebuf_t *sb, int c);
void MSG_WriteLong(sizebuf_t *sb, int c);
void MSG_WriteFloat(sizebuf_t *sb, float f);
void MSG_WriteString(sizebuf_t *sb, const char *s);
void MSG_WriteCoord(sizebuf_t *sb, float f, unsigned int flags);
void MSG_WriteAngle(sizebuf_t *sb, float f, unsigned int flags);
void MSG_WriteAngle16(sizebuf_t *sb, float f, unsigned int flags);
void MSG_BeginReading();
int MSG_ReadChar();
int MSG_ReadByte();
int MSG_ReadShort();
int MSG_ReadLong();
float MSG_ReadFloat();
char *MSG_ReadString();
float MSG_ReadCoord();
float MSG_ReadAngle(unsigned int flags);
float MSG_ReadAngle16 (unsigned int flags);
void Q_memset(void *dest, int fill, size_t count);
void Q_memcpy(void *dest, const void *src, size_t count);
void Q_memmove(void *dest, const void *src, size_t count);
void Q_strcpy(char *dest, const char *src);
void Q_strncpy(char *dest, const char *src, int count);
int Q_strlen(const char *str);
char *Q_strrchr(const char *s, char c);
void Q_strcat(char *dest, const char *src);
int Q_strcmp(const char *s1, const char *s2);
int Q_strncmp(const char *s1, const char *s2, int count);
int Q_strcasecmp(const char *s1, const char *s2);
int Q_strncasecmp(const char *s1, const char *s2, int n);
int Q_atoi(const char *str);
float Q_atof(const char *str);
const char *COM_ParseEx(const char *data, cpe_mode mode);
char *COM_Parse(char *data);
int COM_CheckParm(char *parm);
void COM_Init();
void COM_InitArgv(int argc, char **argv);
void COM_FileBase(char *in, char *out);
void COM_DefaultExtension(char *path, char *extension);
void COM_CreatePath(char *path);
void COM_WriteFile(char *filename, void *data, int len);
int COM_OpenFile(char *filename, int *hndl);
int COM_FOpenFile(char *filename, FILE **file);
void COM_CloseFile(int h);
unsigned char *COM_LoadStackFile(char *path, void *buffer, int bufsize);
unsigned char *COM_LoadHunkFile(char *path);
void COM_LoadCacheFile(char *path, struct cache_user_s *cu);
char *va(char *format, ...); // does a varargs printf into a temp buffer
void SZ_Alloc(sizebuf_t *buf, int startsize);
void SZ_Clear(sizebuf_t *buf);
void *SZ_GetSpace(sizebuf_t *buf, int length);
void SZ_Write(sizebuf_t *buf, void *data, int length);
void SZ_Print(sizebuf_t *buf, char *data); // strcats onto the sizebuf
const char *COM_SkipPath (const char *pathname);
void COM_StripExtension (const char *in, char *out, size_t outsize);
const char *COM_FileGetExtension (const char *in);
const char* LOC_GetString (const char *key);
qboolean LOC_HasPlaceholders (const char *str);
size_t LOC_Format (const char *format, const char* (*getarg_fn) (int idx, void* userdata), void* userdata, char* out, size_t len);
void COM_AddExtension (char *path, const char *extension, size_t len);
byte *COM_LoadHunkFile2 (const char *path, unsigned int *path_id);
byte *COM_LoadFile2 (const char *path, int usehunk, unsigned int *path_id);
byte *COM_LoadStackFile2 (const char *path, void *buffer, int bufsize, unsigned int *path_id);
void COM_FileBase2 (const char *in, char *out, size_t outsize);
int FS_fseek(fshandle_t *fh, long offset, int whence);
size_t FS_fread(void *ptr, size_t size, size_t nmemb, fshandle_t *fh);
int COM_OpenFile2 (const char *filename, int *handle, unsigned int *path_id);
int COM_FOpenFile2 (const char *filename, FILE **file, unsigned int *path_id);
byte *COM_LoadMallocFile (const char *path, unsigned int *path_id);
int FS_fclose(fshandle_t *fh);
