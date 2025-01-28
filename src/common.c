// Copyright (C) 1996-2001 Id Software, Inc.
// Copyright (C) 2002-2009 John Fitzgibbons and others
// GPLv3 See LICENSE for details.

// common.c -- misc functions used in client and server

#include "quakedef.h"

#define NUM_SAFE_ARGVS  7
// if a packfile directory differs from this, it is assumed to be hacked
#define PAK0_COUNT 339
#define PAK0_CRC 32981

qboolean com_modified; // set true if using non-id files
int com_nummissionpacks; //johnfitz
qboolean proghack;
int static_registered = 1; // only for startup check, then set
qboolean msg_suppress_1 = 0;
char com_token[1024];
int com_argc;
char **com_argv;
char com_cmdline[CMDLINE_LENGTH];
qboolean standard_quake = true, rogue, hipnotic;
int msg_readcount;
qboolean msg_badread;
char com_cachedir[MAX_OSPATH];
char com_gamedir[MAX_OSPATH];
int com_filesize;
searchpath_t *com_searchpaths;
cache_user_t *loadcache;
unsigned char *loadbuf;
int loadsize;
extern int file_from_pak;

cvar_t registered = { "registered", "0", false, false, 0, NULL };
cvar_t cmdline = { "cmdline", "", false, true, 0, NULL };

unsigned short pop[] = {
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x6600, 0x0000, 0x0000, 0x0000, 0x6600, 0x0000,
	0x0000, 0x0066, 0x0000, 0x0000, 0x0000, 0x0000, 0x0067, 0x0000,
	0x0000, 0x6665, 0x0000, 0x0000, 0x0000, 0x0000, 0x0065, 0x6600,
	0x0063, 0x6561, 0x0000, 0x0000, 0x0000, 0x0000, 0x0061, 0x6563,
	0x0064, 0x6561, 0x0000, 0x0000, 0x0000, 0x0000, 0x0061, 0x6564,
	0x0064, 0x6564, 0x0000, 0x6469, 0x6969, 0x6400, 0x0064, 0x6564,
	0x0063, 0x6568, 0x6200, 0x0064, 0x6864, 0x0000, 0x6268, 0x6563,
	0x0000, 0x6567, 0x6963, 0x0064, 0x6764, 0x0063, 0x6967, 0x6500,
	0x0000, 0x6266, 0x6769, 0x6a68, 0x6768, 0x6a69, 0x6766, 0x6200,
	0x0000, 0x0062, 0x6566, 0x6666, 0x6666, 0x6666, 0x6562, 0x0000,
	0x0000, 0x0000, 0x0062, 0x6364, 0x6664, 0x6362, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0062, 0x6662, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0061, 0x6661, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x6500, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x6400, 0x0000, 0x0000, 0x0000
}; // this graphic needs to be in the pak file to use registered features

static char *largv[MAX_NUM_ARGVS + NUM_SAFE_ARGVS + 1];
static char *argvdummy = " ";
static char *safeargvs[NUM_SAFE_ARGVS] =
    {"-stdvid","-nolan","-nosound","-nocdaudio","-nojoy","-nomouse","-dibonly"};

void COM_InitFilesystem();
void COM_Path_f();

/*
All of Quake's data access is through a hierchal file system, but the contents
of the file system can be transparently merged from several sources.

The "base directory" is the path to the directory holding the quake.exe and all
game directories. sys_* files pass this to host_init in quakeparms_t->basedir.
This can be overridden with the "-basedir" command line parm to allow code
debugging in a different directory.  The base directory is only used during
filesystem initialization.

The "game directory" is the first tree on the search path and directory that all
generated files (savegames, screenshots, demos, config files) will be saved to.
This can be overridden with the "-game" command line parameter. The game dir can
never be changed while quake is executing. This is a precacution against having
a malicious server instruct clients to write files over areas they shouldn't.

The "cache directory" is only used during development to save network bandwidth,
especially over ISDN / T1 lines.  If there is a cache directory specified, when
a file is found by the normal search path, it will be mirrored into the cache
directory, then opened there.

FIXME:
The file "parms.txt" will be read out of the game directory and appended to the
current command line arguments to allow different games to initialize startup
parms differently.  This could be used to add a "-sspeed 22050" for the high
quality sound edition.  Because they are added at the end, they will not
override an explicit setting on the original command line.
*/

typedef struct
{
        char *key;
        char *value;
} locentry_t;

typedef struct
{
        int                     numentries;
        int                     maxnumentries;
        int                     numindices;
        unsigned        *indices;
        locentry_t      *entries;
        char            *text;
} localization_t;

static localization_t localization;

static inline int q_isdigit(int c)
{
        return (c >= '0' && c <= '9');
}

/*
================
LOC_ParseArg

Returns argument index (>= 0) and advances the string if it starts with a placeholder ({} or {N}),
otherwise returns a negative value and leaves the pointer unchanged
================
*/
static int LOC_ParseArg (const char **pstr)
{
        int arg;
        const char *str = *pstr;

        // opening brace
        if (*str != '{')
                return -1;
        ++str;

        // optional index, defaulting to 0
        arg = 0;
        while (q_isdigit(*str))
                arg = arg * 10 + *str++ - '0';

        // closing brace
        if (*str != '}')
                return -1;
        *pstr = ++str;

        return arg;
}

/*
================
LOC_HasPlaceholders
================
*/
qboolean LOC_HasPlaceholders (const char *str)
{
        if (!localization.numindices)
                return false;
        while (*str)
        {
                if (LOC_ParseArg(&str) >= 0)
                        return true;
                str++;
        }
        return false;
}

/*
================
COM_HashString
Computes the FNV-1a hash of string str
================
*/
unsigned COM_HashString (const char *str)
{
        unsigned hash = 0x811c9dc5u;
        while (*str)
        {
                hash ^= *str++;
                hash *= 0x01000193u;
        }
        return hash;
}

/*
================
LOC_GetRawString

Returns localized string if available, or NULL otherwise
================
*/
const char* LOC_GetRawString (const char *key)
{
        unsigned pos, end;

        if (!localization.numindices || !key || !*key || *key != '$')
                return NULL;
        key++;

        pos = COM_HashString(key) % localization.numindices;
        end = pos;

        do
        {
                unsigned idx = localization.indices[pos];
                locentry_t *entry;
                if (!idx)
                        return NULL;

                entry = &localization.entries[idx - 1];
                if (!Q_strcmp(entry->key, key))
                        return entry->value;

                ++pos;
                if (pos == localization.numindices)
                        pos = 0;
        } while (pos != end);

        return NULL;
}

/*
================
LOC_GetString

Returns localized string if available, or input string otherwise
================
*/
const char* LOC_GetString (const char *key)
{
        const char* value = LOC_GetRawString(key);
        return value ? value : key;
}

void ClearLink(link_t *l) // ClearLink is used for new headnodes
{
	l->prev = l->next = l;
}

void RemoveLink(link_t *l)
{
	l->next->prev = l->prev;
	l->prev->next = l->next;
}

void InsertLinkBefore(link_t *l, link_t *before)
{
	l->next = before;
	l->prev = before->prev;
	l->prev->next = l;
	l->next->prev = l;
}

void InsertLinkAfter(link_t *l, link_t *after)
{
	l->next = after->next;
	l->prev = after;
	l->prev->next = l;
	l->next->prev = l;
}

size_t FS_fread(void *ptr, size_t size, size_t nmemb, fshandle_t *fh)
{
        long byte_size;
        long bytes_read;
        size_t nmemb_read;

        if (!fh) {
                errno = EBADF;
                return 0;
        }
        if (!ptr) {
                errno = EFAULT;
                return 0;
        }
        if (!size || !nmemb) {  /* no error, just zero bytes wanted */
                errno = 0;
                return 0;
        }

        byte_size = nmemb * size;
        if (byte_size > fh->length - fh->pos)   /* just read to end */
                byte_size = fh->length - fh->pos;
        bytes_read = fread(ptr, 1, byte_size, fh->file);
        fh->pos += bytes_read;

        /* fread() must return the number of elements read,
         * not the total number of bytes. */
        nmemb_read = bytes_read / size;
        /* even if the last member is only read partially
         * it is counted as a whole in the return value. */
        if (bytes_read % size)
                nmemb_read++;

        return nmemb_read;
}

int FS_fseek(fshandle_t *fh, long offset, int whence)
{
/* I don't care about 64 bit off_t or fseeko() here.
 * the quake/hexen2 file system is 32 bits, anyway. */
        int ret;

        if (!fh) {
                errno = EBADF;
                return -1;
        }

        /* the relative file position shouldn't be smaller
         * than zero or bigger than the filesize. */
        switch (whence)
        {
        case SEEK_SET:
                break;
        case SEEK_CUR:
                offset += fh->pos;
                break;
        case SEEK_END:
                offset = fh->length + offset;
                break;
        default:
                errno = EINVAL;
                return -1;
        }

        if (offset < 0) {
                errno = EINVAL;
                return -1;
        }

        if (offset > fh->length)        /* just seek to end */
                offset = fh->length;

        ret = fseek(fh->file, fh->start + offset, SEEK_SET);
        if (ret < 0)
                return ret;

        fh->pos = offset;
        return 0;
}


// Library Replacement Functions
// CyanBun96: the standard functions should be faster and more reliable, and
// the benefits of replacing them are not there three decades later.
// The prototypes are kept the same for compatibility, some non-standard
// behavior is preserved (like the return value of strcmp)
void Q_memset(void *dest, int fill, size_t count)
{
	memset(dest, fill, count);
}

void Q_memmove(void *dest, const void *src, size_t count)
{
	memmove(dest, src, count);
}

void Q_memcpy(void *dest, const void *src, size_t count)
{
	memcpy(dest, src, count);
}

void Q_strcpy(char *dest, const char *src)
{
	strcpy(dest, src);
}

void Q_strncpy(char *dest, const char *src, int count)
{
	strncpy(dest, src, count);
}

int Q_strlen(const char *str)
{
	return strlen(str);
}

char *Q_strrchr(const char *s, char c)
{
	return strrchr(s, c);
}

void Q_strcat(char *dest, const char *src)
{
	strcat(dest, src);
}

int Q_strcmp(const char *s1, const char *s2)
{
	return strcmp(s1, s2) == 0 ? 0 : -1;
}

int Q_strncmp(const char *s1, const char *s2, int count)
{
	return strncmp(s1, s2, count) == 0 ? 0 : -1;
}

int Q_strncasecmp(const char *s1, const char *s2, int n)
{
	return strncasecmp(s1, s2, n) == 0 ? 0 : -1;
}

int Q_strcasecmp(const char *s1, const char *s2)
{
	return strcasecmp(s1, s2) == 0 ? 0 : -1;
}

int Q_atoi(const char *str)
{
	int sign = 1;
	if (*str == '-') {
		sign = -1;
		str++;
	}
	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) // Check for hex
		return sign * (int)strtol(str, NULL, 16);
	if (str[0] == '\'' && str[1] != '\0') // Check for character constant
		return sign * (int)str[1];
	return sign * atoi(str); // Assume decimal
}

float Q_atof(const char *str)
{
	int sign = 1;
	if (*str == '-') {
		sign = -1;
		str++;
	}
	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) // Check for hex
		return (float)(sign * strtod(str, NULL));
	if (str[0] == '\'' && str[1] != '\0') // Check for character constant
		return (float)(sign * (int)str[1]);
	return (float)(sign * strtod(str, NULL)); // Assume decimal
}

// Message IO Functions
// Handles byte ordering and avoids alignment errors
// CyanBun96: TODO replace with something more modern and less fiddly?
void MSG_WriteChar(sizebuf_t *sb, int c) // writing functions ------------------
{
	char *buf = SZ_GetSpace(sb, 1);
	buf[0] = c;
}

void MSG_WriteByte(sizebuf_t *sb, int c)
{
	unsigned char *buf = SZ_GetSpace(sb, 1);
	buf[0] = c;
}

void MSG_WriteShort(sizebuf_t *sb, int c)
{
	unsigned char *buf = SZ_GetSpace(sb, 2);
	buf[0] = c & 0xff;
	buf[1] = c >> 8;
}

void MSG_WriteLong(sizebuf_t *sb, int c)
{
	unsigned char *buf = SZ_GetSpace(sb, 4);
	buf[0] = c & 0xff;
	buf[1] = (c >> 8) & 0xff;
	buf[2] = (c >> 16) & 0xff;
	buf[3] = c >> 24;
}

void MSG_WriteFloat(sizebuf_t *sb, float f)
{
	union {
		float f;
		int l;
	} dat;
	dat.f = f;
	dat.l = LittleLong(dat.l);
	SZ_Write(sb, &dat.l, 4);
}

void MSG_WriteString(sizebuf_t *sb, const char *s)
{
	if (!s)
		SZ_Write(sb, "", 1);
	else
		SZ_Write(sb, s, Q_strlen(s) + 1);
}

void MSG_WriteCoord16(sizebuf_t *sb, float f)
{ //johnfitz -- original behavior, 13.3 fixed point coords, max range +-4096
	MSG_WriteShort(sb, Q_rint(f * 8));
}

void MSG_WriteCoord24 (sizebuf_t *sb, float f)
{
        MSG_WriteShort (sb, f);
        MSG_WriteByte (sb, (int)(f*255)%255);
}

void MSG_WriteCoord (sizebuf_t *sb, float f, unsigned int flags)
{
        if (flags & PRFL_FLOATCOORD)
                MSG_WriteFloat (sb, f);
        else if (flags & PRFL_INT32COORD)
                MSG_WriteLong (sb, Q_rint (f * 16));
        else if (flags & PRFL_24BITCOORD)
                MSG_WriteCoord24 (sb, f);
        else MSG_WriteCoord16 (sb, f);
}

void MSG_WriteAngle (sizebuf_t *sb, float f, unsigned int flags)
{
        if (flags & PRFL_FLOATANGLE)
                MSG_WriteFloat (sb, f);
        else if (flags & PRFL_SHORTANGLE)
                MSG_WriteShort (sb, Q_rint(f * 65536.0 / 360.0) & 65535);
        else MSG_WriteByte (sb, Q_rint(f * 256.0 / 360.0) & 255); //johnfitz -- use Q_rint instead of (int)     }
}

//johnfitz -- for PROTOCOL_FITZQUAKE
void MSG_WriteAngle16 (sizebuf_t *sb, float f, unsigned int flags)
{
        if (flags & PRFL_FLOATANGLE)
                MSG_WriteFloat (sb, f);
        else MSG_WriteShort (sb, Q_rint(f * 65536.0 / 360.0) & 65535);
}

void MSG_BeginReading() // Reading Functions ------------------
{
	msg_readcount = 0;
	msg_badread = false;
}

int MSG_ReadChar()
{ // returns -1 and sets msg_badread if no more characters are available
	if (msg_readcount + 1 > net_message.cursize) {
		msg_badread = true;
		return -1;
	}
	return (char)net_message.data[msg_readcount++];
}

int MSG_ReadByte()
{
	if (msg_readcount + 1 > net_message.cursize) {
		msg_badread = true;
		return -1;
	}
	return (unsigned char)net_message.data[msg_readcount++];
}

int MSG_ReadShort()
{
	if (msg_readcount + 2 > net_message.cursize) {
		msg_badread = true;
		return -1;
	}
	int c = (short)(net_message.data[msg_readcount]
			+ (net_message.data[msg_readcount + 1] << 8));
	msg_readcount += 2;
	return c;
}

int MSG_ReadLong()
{
	if (msg_readcount + 4 > net_message.cursize) {
		msg_badread = true;
		return -1;
	}
	int c = net_message.data[msg_readcount]
	    + (net_message.data[msg_readcount + 1] << 8)
	    + (net_message.data[msg_readcount + 2] << 16)
	    + (net_message.data[msg_readcount + 3] << 24);
	msg_readcount += 4;
	return c;
}

float MSG_ReadFloat()
{
	union {
		byte b[4];
		float f;
		int l;
	} dat;
	dat.b[0] = net_message.data[msg_readcount];
	dat.b[1] = net_message.data[msg_readcount + 1];
	dat.b[2] = net_message.data[msg_readcount + 2];
	dat.b[3] = net_message.data[msg_readcount + 3];
	msg_readcount += 4;
	dat.l = LittleLong(dat.l);
	return dat.f;
}

char *MSG_ReadString()
{
	static char string[2048];
	unsigned long l = 0;
	do {
		int c = MSG_ReadChar();
		if (c == -1 || c == 0)
			break;
		string[l] = c;
		l++;
	} while (l < sizeof(string) - 1);
	string[l] = 0;
	return string;
}

float MSG_ReadCoord16()
{ //johnfitz -- original behavior, 13.3 fixed point coords, max range +-4096
	return MSG_ReadShort() * (1.0 / 8);
}

float MSG_ReadCoord24()
{
        return MSG_ReadShort() + MSG_ReadByte() * (1.0/255);
}

float MSG_ReadCoord(unsigned int flags)
{
	if (flags & PRFL_FLOATCOORD)
		return MSG_ReadFloat ();
	else if (flags & PRFL_INT32COORD)
		return MSG_ReadLong () * (1.0 / 16.0);
	else if (flags & PRFL_24BITCOORD)
		return MSG_ReadCoord24 ();
	else return MSG_ReadCoord16 ();
}

float MSG_ReadAngle(unsigned int flags)
{
	if (flags & PRFL_FLOATANGLE)
		return MSG_ReadFloat ();
	else if (flags & PRFL_SHORTANGLE)
		return MSG_ReadShort () * (360.0 / 65536);
	else return MSG_ReadChar () * (360.0 / 256);
}

float MSG_ReadAngle16 (unsigned int flags)
{
        if (flags & PRFL_FLOATANGLE)
                return MSG_ReadFloat ();        // make sure
        else return MSG_ReadShort () * (360.0 / 65536);
}

void SZ_Alloc(sizebuf_t *buf, int startsize)
{
	if (startsize < 256)
		startsize = 256;
	buf->data = Hunk_AllocName(startsize, "sizebuf");
	buf->maxsize = startsize;
	buf->cursize = 0;
}

void SZ_Clear(sizebuf_t *buf)
{
	buf->cursize = 0;
}

void *SZ_GetSpace(sizebuf_t *buf, int length)
{
	if (buf->cursize + length > buf->maxsize) {
		if (!buf->allowoverflow)
			Sys_Error
			    ("SZ_GetSpace: overflow without allowoverflow set");
		if (length > buf->maxsize)
			Sys_Error("SZ_GetSpace: %i is > full buffer size",
				  length);
		buf->overflowed = true;
		Con_Printf("SZ_GetSpace: overflow");
		SZ_Clear(buf);
	}
	void *data = buf->data + buf->cursize;
	buf->cursize += length;
	return data;
}

void SZ_Write(sizebuf_t *buf, void *data, int length)
{
	Q_memcpy(SZ_GetSpace(buf, length), data, length);
}

void SZ_Print(sizebuf_t *buf, char *data)
{
	int len = Q_strlen(data) + 1;
	if (buf->data[buf->cursize - 1]) // no trailing 0
		Q_memcpy(SZ_GetSpace(buf, len), data, len);
	else // write over trailing 0
		Q_memcpy(SZ_GetSpace(buf, len - 1) - 1, data, len);
}

const char *COM_SkipPath (const char *pathname)
{
        const char      *last;

        last = pathname;
        while (*pathname)
        {
                if (*pathname == '/')
                        last = pathname + 1;
                pathname++;
        }
        return last;
}

void COM_StripExtension (const char *in, char *out, size_t outsize)
{
        int     length;

        if (!*in)
        {
                *out = '\0';
                return;
        }
        if (in != out)  /* copy when not in-place editing */
                strlcpy (out, in, outsize);
        length = (int)strlen(out) - 1;
        while (length > 0 && out[length] != '.')
        {
                --length;
                if (out[length] == '/' || out[length] == '\\')
                        return; /* no extension */
        }
        if (length > 0)
                out[length] = '\0';
}

const char *COM_FileGetExtension (const char *in)
{ // COM_FileGetExtension - doesn't return NULL
        const char      *src;
        size_t          len;

        len = strlen(in);
        if (len < 2)    /* nothing meaningful */
                return "";

        src = in + len - 1;
        while (src != in && src[-1] != '.')
                src--;
        if (src == in || strchr(src, '/') != NULL || strchr(src, '\\') != NULL)
                return "";      /* no extension, or parent directory has a dot */

        return src;
}

char *COM_FileExtension(char *in)
{
	static char exten[8];
	while (*in && *in != '.')
		in++;
	if (!*in)
		return "";
	in++;
	int i;
	for (i = 0; i < 7 && *in; i++, in++)
		exten[i] = *in;
	exten[i] = 0;
	return exten;
}

void COM_FileBase(char *in, char *out)
{
	char *s = in + strlen(in) - 1;
	while (s != in && *s != '.')
		s--;
	char *s2;
	for (s2 = s; s2 != in && *s2 && *s2 != '/'; s2--) ;
	if (s - s2 < 2)
		strcpy(out, "?model?");
	else {
		s--;
		strncpy(out, s2 + 1, s - s2);
		out[s - s2] = 0;
	}
}

void COM_DefaultExtension(char *path, char *extension)
{ // If path doesn't have a .EXT, append extension
  // (extension should include the .)
	char *src = path + strlen(path) - 1;
	while (*src != '/' && src != path) {
		if (*src == '.')
			return;	// it has an extension
		src--;
	}
	strcat(path, extension);
}

/*
==============
COM_ParseEx

Parse a token out of a string

The mode argument controls how overflow is handled:
- CPE_NOTRUNC:          return NULL (abort parsing)
- CPE_ALLOWTRUNC:       truncate com_token (ignore the extra characters in this token)
==============
*/
const char *COM_ParseEx (const char *data, cpe_mode mode)
{
        int             c;
        int             len;

        len = 0;
        com_token[0] = 0;

        if (!data)
                return NULL;

// skip whitespace
skipwhite:
        while ((c = *data) <= ' ')
        {
                if (c == 0)
                        return NULL;    // end of file
                data++;
        }

// skip // comments
        if (c == '/' && data[1] == '/')
        {
                while (*data && *data != '\n')
                        data++;
                goto skipwhite;
        }

// skip /*..*/ comments
        if (c == '/' && data[1] == '*')
        {
                data += 2;
                while (*data && !(*data == '*' && data[1] == '/'))
                        data++;
                if (*data)
                        data += 2;
                goto skipwhite;
        }

// handle quoted strings specially
        if (c == '\"')
        {
                data++;
                while (1)
                {
                        if ((c = *data) != 0)
                                ++data;
                        if (c == '\"' || !c)
                        {
                                com_token[len] = 0;
                                return data;
                        }
                        if (len < Q_COUNTOF(com_token) - 1)
                                com_token[len++] = c;
                        else if (mode == CPE_NOTRUNC)
                                return NULL;
                }
        }

// parse single characters
        if (c == '{' || c == '}'|| c == '('|| c == ')' || c == '\'' || c == ':')
        {
                if (len < Q_COUNTOF(com_token) - 1)
                        com_token[len++] = c;
                else if (mode == CPE_NOTRUNC)
                        return NULL;
                com_token[len] = 0;
                return data+1;
        }

// parse a regular word
        do
        {
                if (len < Q_COUNTOF(com_token) - 1)
                        com_token[len++] = c;
                else if (mode == CPE_NOTRUNC)
                        return NULL;
                data++;
                c = *data;
                /* commented out the check for ':' so that ip:port works */
                if (c == '{' || c == '}'|| c == '('|| c == ')' || c == '\''/* || c == ':' */)
                        break;
        } while (c > 32);

        com_token[len] = 0;
        return data;
}

char *COM_Parse(char *data)
{ // Parse a token out of a string
	int c;
	int len = 0;
	com_token[0] = 0;
	if (!data)
		return NULL;
skipwhite: // skip whitespace
	while ((c = *data) <= ' ') {
		if (c == 0)
			return NULL; // end of file;
		data++;
	}
	if (c == '/' && data[1] == '/') { // skip // comments
		while (*data && *data != '\n')
			data++;
		goto skipwhite;
	}
	if (c == '\"') { // handle quoted strings specially
		data++;
		while (1) {
			c = *data++;
			if (c == '\"' || !c) {
				com_token[len] = 0;
				return data;
			}
			com_token[len] = c;
			len++;
		}
	}
	if (c == '{'||c == '}'||c == ')'||c == '('||c == '\''||c == ':') {
		com_token[len] = c; // parse single characters
		len++;
		com_token[len] = 0;
		return data + 1;
	}
	do { // parse a regular word
		com_token[len] = c;
		data++;
		len++;
		c = *data;
		if (c == '{'||c == '}'||c == ')'||c == '('||c == '\''||c == ':')
			break;
	} while (c > 32);
	com_token[len] = 0;
	return data;
}


int COM_CheckParm(char *parm)
{ // Returns the position (1 to argc-1) in the program's argument list
  // where the given parameter apears, or 0 if not present
	for (int i = 1; i < com_argc; i++) {
		if (!com_argv[i])
			continue;
		if (!Q_strcmp(parm, com_argv[i]))
			return i;
	}
	return 0;
}

void COM_CheckRegistered()
{ // Looks for the pop.txt file and verifies it. Sets the "registered" cvar.
	unsigned short check[128];
	int h;
	COM_OpenFile("gfx/pop.lmp", &h);
	static_registered = 0;
	if (h == -1) {
		Con_Printf("Playing shareware version.\n");
		return;
	}
	Sys_FileRead(h, check, sizeof(check));
	COM_CloseFile(h);
	for (int i = 0; i < 128; i++)
		if (pop[i] != (unsigned short)BigShort(check[i]))
			Sys_Error("Corrupted data file.");
	//johnfitz -- eliminate leading space
	Cvar_Set("cmdline", com_cmdline + 1);	
	Cvar_Set("registered", "1");
	static_registered = 1;
	Con_Printf("Playing registered version.\n");
}

void COM_InitArgv(int argc, char **argv)
{ // reconstitute the command line for the cmdline externally visible cvar
  // FIXME This leaks environment vars to stdout, ./quake -1 -2 -3 to reproduce
	int n = 0;
	for (int j = 0; (j < MAX_NUM_ARGVS) && (j < argc); j++) {
		int i = 0;
		while ((n < (CMDLINE_LENGTH - 1)) && argv[j][i])
			com_cmdline[n++] = argv[j][i++];
		if (n < (CMDLINE_LENGTH - 1))
			com_cmdline[n++] = ' ';
		else
			break;
	}
	if (n > 0 && com_cmdline[n - 1] == ' ')
		com_cmdline[n - 1] = 0;	// johnfitz -- kill the trailing space
	Con_Printf("\nCommand line: %s\n", com_cmdline);
	qboolean safe = false;
	for (com_argc = 0; (com_argc < MAX_NUM_ARGVS) && (com_argc < argc);
	     com_argc++) {
		largv[com_argc] = argv[com_argc];
		if (!Q_strcmp("-safe", argv[com_argc]))
			safe = true;
	}
	if (safe) {
		// force all the safe-mode switches. Note that we reserved extra
		// space in case we need to add these, so we don't need an
		// overflow check
		for (int i = 0; i < NUM_SAFE_ARGVS; i++) {
			largv[com_argc] = safeargvs[i];
			com_argc++;
		}
	}
	largv[com_argc] = argvdummy;
	com_argv = largv;
	if (COM_CheckParm("-rogue")) {
		rogue = true;
		standard_quake = false;
	}
	if (COM_CheckParm("-hipnotic") || COM_CheckParm("-quoth"))
	{ //johnfitz -- "-quoth" support
		hipnotic = true;
		standard_quake = false;
	}
}

void COM_Init()
{
	Cvar_RegisterVariable(&registered);
	Cvar_RegisterVariable(&cmdline);
	Cmd_AddCommand("path", COM_Path_f);
	COM_InitFilesystem();
	COM_CheckRegistered();
}

char *va(char *format, ...)
{ // does a varargs printf into a temp buffer, so I don't need to have
  // varargs versions of all text functions.
  // FIXME: make this buffer size safe someday
	va_list argptr;
	static char string[1024];
	va_start(argptr, format);
	vsprintf(string, format, argptr);
	va_end(argptr);
	return string;
}

// Quake Filesystem
void COM_Path_f()
{
	Con_Printf("Current search path:\n");
	for (searchpath_t *s = com_searchpaths; s; s = s->next) {
		if (s->pack) {
			Con_Printf("%s (%i files)\n", s->pack->filename,
				   s->pack->numfiles);
		} else
			Con_Printf("%s\n", s->filename);
	}
}

void COM_WriteFile(char *filename, void *data, int len)
{ // The filename will be prefixed by the current game directory
	char name[MAX_OSPATH + 1];
	Sys_mkdir(com_gamedir);	// johnfitz -- if we've switched to a 
		// nonexistant gamedir, create it now so we don't crash
	sprintf(name, "%s/%s", com_gamedir, filename);
	int handle = Sys_FileOpenWrite(name);
	if (handle == -1) {
		Sys_Printf("COM_WriteFile: failed on %s\n", name);
		return;
	}
	Sys_Printf("COM_WriteFile: %s\n", name);
	Sys_FileWrite(handle, data, len);
	Sys_FileClose(handle);
}

void COM_CreatePath(char *path)
{
	for (char *ofs = path + 1; *ofs; ofs++) {
		if (*ofs == '/') { // create the directory
			*ofs = 0;
			Sys_mkdir(path);
			*ofs = '/';
		}
	}
}

void COM_CopyFile(char *netpath, char *cachepath)
{ // Copies a file over from the net to the local cache, creating any needed
  // directories. This is for the convenience of developers using ISDN from home
	int in;
	unsigned long remaining = Sys_FileOpenRead(netpath, &in);
	COM_CreatePath(cachepath); // create directories up to the cache file
	int out = Sys_FileOpenWrite(cachepath);
	char buf[4096];
	unsigned long count;
	while (remaining) {
		count = remaining < sizeof(buf) ? remaining : sizeof(buf);
		Sys_FileRead(in, buf, count);
		Sys_FileWrite(out, buf, count);
		remaining -= count;
	}
	Sys_FileClose(in);
	Sys_FileClose(out);
}

int COM_FindFile(char *filename, int *handle, FILE **file)
{ // Finds file in the search path. Sets com_filesize and one of handle or file
	char netpath[MAX_OSPATH + 1];
	char cachepath[MAX_OSPATH * 2];
	int i;
	if (file && handle)
		Sys_Error("COM_FindFile: both handle and file set");
	if (!file && !handle)
		Sys_Error("COM_FindFile: neither handle or file set");
	// search through the path, one element at a time
	searchpath_t *search = com_searchpaths;
	if (proghack) // gross hack to use quake 1 progs with quake 2 maps
		if (!strcmp(filename, "progs.dat"))
			search = search->next;
	for (; search; search = search->next) {
		if (search->pack) { // is the element a pak file?
			// look through all the pak file elements
			pack_t *pak = search->pack;
			for (i = 0; i < pak->numfiles; i++)
				if (!strcmp(pak->files[i].name, filename)) { // found it!
					Con_DPrintf("PackFile: %s : %s\n", pak->filename, filename);
					if (handle) {
						*handle = pak->handle;
						Sys_FileSeek(pak->handle, pak->files[i].filepos);
					} else { // open a new file on the pakfile
						*file = fopen(pak->filename, "rb");
						if (*file)
							fseek(*file, pak->files[i]. filepos, SEEK_SET);
					}
					com_filesize = pak->files[i].filelen;
					return com_filesize;
				}
		} else {
			// check a file in the directory tree
			if (!static_registered) { // if not a registered version, don't ever go beyond base
				if (strchr(filename, '/')
				    || strchr(filename, '\\'))
					continue;
			}
			sprintf(netpath, "%s/%s", search->filename, filename);
			int findtime = Sys_FileTime(netpath);
			if (findtime == -1)
				continue;
			// see if the file needs to be updated in the cache
			if (!com_cachedir[0])
				strcpy(cachepath, netpath);
			else {
#ifdef _WIN32
				if ((strlen(netpath) < 2)
				    || (netpath[1] != ':'))
					sprintf(cachepath, "%s%s", com_cachedir,
						netpath);
				else
					sprintf(cachepath, "%s%s", com_cachedir,
						netpath + 2);
#else
				sprintf(cachepath, "%s%s", com_cachedir,
					netpath);
#endif
				int cachetime = Sys_FileTime(cachepath);
				if (cachetime < findtime)
					COM_CopyFile(netpath, cachepath);
				strcpy(netpath, cachepath);
			}
			Con_DPrintf("FindFile: %s\n", netpath);
			com_filesize = Sys_FileOpenRead(netpath, &i);
			if (handle)
				*handle = i;
			else {
				Sys_FileClose(i);
				*file = fopen(netpath, "rb");
			}
			return com_filesize;
		}
	}
	Con_DPrintf("FindFile: can't find %s\n", filename);
	if (handle)
		*handle = -1;
	else
		*file = NULL;
	com_filesize = -1;
	return -1;
}

int COM_OpenFile(char *filename, int *handle)
{ // filename never has a leading slash, but may contain directory walks
  // returns a handle and a length, it may actually be inside a pak file
	return COM_FindFile(filename, handle, NULL);
}

int COM_FOpenFile(char *filename, FILE **file)
{ // If the requested file is inside a packfile, a new FILE * will be opened
  // into the file.
	return COM_FindFile(filename, NULL, file);
}

void COM_CloseFile(int h)
{ // If it is a pak file handle, don't really close it
	for (searchpath_t *s = com_searchpaths; s; s = s->next)
		if (s->pack && s->pack->handle == h)
			return;
	Sys_FileClose(h);
}

unsigned char *COM_LoadFile(char *path, int usehunk)
{ // Filenames are reletive to the quake directory. Allways appends a 0 byte.
	// look for it in the filesystem or pack files
	int h;
	int len = COM_OpenFile(path, &h);
	if (h == -1)
		return NULL;
	// extract the filename base name for hunk tag
	char base[32];
	COM_FileBase(path, base);
	unsigned char *buf = NULL; // quiet compiler warning
	if (usehunk == 1)
		buf = Hunk_AllocName(len + 1, base);
	else if (usehunk == 2)
		buf = Hunk_TempAlloc(len + 1);
	else if (usehunk == 0)
		buf = Z_Malloc(len + 1);
	else if (usehunk == 3)
		buf = Cache_Alloc(loadcache, len + 1, base);
	else if (usehunk == 4) {
		if (len + 1 > loadsize)
			buf = Hunk_TempAlloc(len + 1);
		else
			buf = loadbuf;
	} else
		Sys_Error("COM_LoadFile: bad usehunk");
	if (!buf)
		Sys_Error("COM_LoadFile: not enough space for %s", path);
	buf[len] = 0;
	Sys_FileRead(h, buf, len);
	COM_CloseFile(h);
	return buf;
}

long COM_filelength (FILE *f)
{
        long            pos, end;

        pos = ftell (f);
        fseek (f, 0, SEEK_END);
        end = ftell (f);
        fseek (f, pos, SEEK_SET);

        return end;
}

/*
===========
COM_FindFile

Finds the file in the search path.
Sets com_filesize and one of handle or file
If neither of file or handle is set, this
can be used for detecting a file's presence.
===========
*/
static int COM_FindFile2 (const char *filename, int *handle, FILE **file,
                                                        unsigned int *path_id)
{
        searchpath_t    *search;
        char            netpath[MAX_OSPATH];
        pack_t          *pak;
        int             i;

        if (file && handle)
                Sys_Error ("COM_FindFile: both handle and file set");

        file_from_pak = 0;

//
// search through the path, one element at a time
//
        for (search = com_searchpaths; search; search = search->next)
        {
                if (search->pack)       /* look through all the pak file elements */
                {
                        pak = search->pack;
                        for (i = 0; i < pak->numfiles; i++)
                        {
                                if (strcmp(pak->files[i].name, filename) != 0)
                                        continue;
                                // found it!
                                com_filesize = pak->files[i].filelen;
                                file_from_pak = 1;
                                if (path_id)
                                        *path_id = search->path_id;
                                if (handle)
                                {
                                        *handle = pak->handle;
                                        Sys_FileSeek (pak->handle, pak->files[i].filepos);
                                        return com_filesize;
                                }
                                else if (file)
                                { /* open a new file on the pakfile */
                                        *file = fopen (pak->filename, "rb");
                                        if (*file)
                                                fseek (*file, pak->files[i].filepos, SEEK_SET);
                                        return com_filesize;
                                }
                                else /* for COM_FileExists() */
                                {
                                        return com_filesize;
                                }
                        }
                }
                else    /* check a file in the directory tree */
                {
                        if (!registered.value)
                        { /* if not a registered version, don't ever go beyond base */
                                if ( strchr (filename, '/') || strchr (filename,'\\'))
                                        continue;
                        }

                        snprintf (netpath, sizeof(netpath), "%s/%s",search->filename, filename);
                        //if (! (Sys_FileType(netpath) & FS_ENT_FILE))
                        //FIXME        continue;

                        if (path_id)
                                *path_id = search->path_id;
                        if (handle)
                        {
                                com_filesize = Sys_FileOpenRead (netpath, &i);
                                *handle = i;
                                return com_filesize;
                        }
                        else if (file)
                        {
                                *file = fopen (netpath, "rb");
                                com_filesize = (*file == NULL) ? -1 : COM_filelength (*file);
                                return com_filesize;
                        }
                        else
                        {
                                return 0; /* dummy valid value for COM_FileExists() */
                        }
                }
        }

        if (strcmp(COM_FileGetExtension(filename), "pcx") != 0
                && strcmp(COM_FileGetExtension(filename), "tga") != 0
                && strcmp(COM_FileGetExtension(filename), "lit") != 0
                && strcmp(COM_FileGetExtension(filename), "vis") != 0
                && strcmp(COM_FileGetExtension(filename), "ent") != 0)
                printf ("FindFile: can't find %s\n", filename);
        else    printf("FindFile: can't find %s\n", filename);

        if (handle)
                *handle = -1;
        if (file)
                *file = NULL;
        com_filesize = -1;
        return com_filesize;
}

void COM_FileBase2 (const char *in, char *out, size_t outsize)
{
        const char      *dot, *slash, *s;

        s = in;
        slash = in;
        dot = NULL;
        while (*s)
        {
                if (*s == '/' || *s == '\\')
                        slash = s + 1;
                if (*s == '.')
                        dot = s;
                s++;
        }
        if (dot == NULL)
                dot = s;

        if (dot - slash < 2)
                strlcpy (out, "?model?", outsize);
        else
        {
                size_t  len = dot - slash;
                if (len >= outsize)
                        len = outsize - 1;
                memcpy (out, slash, len);
                out[len] = '\0';
        }
}

byte *COM_LoadStackFile2 (const char *path, void *buffer, int bufsize, unsigned int *path_id)
{
        byte    *buf;

        loadbuf = (byte *)buffer;
        loadsize = bufsize;
        buf = COM_LoadFile2 (path, LOADFILE_STACK, path_id);

        return buf;
}

int FS_fclose(fshandle_t *fh)
{
        if (!fh) {
                errno = EBADF;
                return -1;
        }
        return fclose(fh->file);
}

byte *COM_LoadMallocFile (const char *path, unsigned int *path_id)
{
        return COM_LoadFile2 (path, LOADFILE_MALLOC, path_id);
}

int COM_FOpenFile2 (const char *filename, FILE **file, unsigned int *path_id)
{
        return COM_FindFile2 (filename, NULL, file, path_id);
}

int COM_OpenFile2 (const char *filename, int *handle, unsigned int *path_id)
{
        return COM_FindFile2 (filename, handle, NULL, path_id);
}

byte *COM_LoadFile2 (const char *path, int usehunk, unsigned int *path_id)
{
        int             h;
        byte    *buf;
        char    base[32];
        int     len, nread;

        buf = NULL;     // quiet compiler warning

// look for it in the filesystem or pack files
        len = COM_OpenFile2 (path, &h, path_id);
        if (h == -1)
                return NULL;

// extract the filename base name for hunk tag
        COM_FileBase2 (path, base, sizeof(base));

        switch (usehunk)
        {
        case LOADFILE_HUNK:
                buf = (byte *) Hunk_AllocName (len+1, base);
                break;
        case LOADFILE_TEMPHUNK:
                buf = (byte *) Hunk_TempAlloc (len+1);
                break;
        case LOADFILE_ZONE:
                buf = (byte *) Z_Malloc (len+1);
                break;
        case LOADFILE_CACHE:
                buf = (byte *) Cache_Alloc (loadcache, len+1, base);
                break;
        case LOADFILE_STACK:
                if (len < loadsize)
                        buf = loadbuf;
                else
                        buf = (byte *) Hunk_TempAlloc (len+1);
                break;
        case LOADFILE_MALLOC:
                buf = (byte *) malloc (len+1);
                break;
        default:
                Sys_Error ("COM_LoadFile: bad usehunk");
        }

        if (!buf)
                Sys_Error ("COM_LoadFile: not enough space for %s", path);

        ((byte *)buf)[len] = 0;

        nread = Sys_FileRead (h, buf, len);
        COM_CloseFile (h);
        if (nread != len)
                Sys_Error ("COM_LoadFile: Error reading %s", path);

        return buf;
}

unsigned char *COM_LoadHunkFile(char *path)
{
	return COM_LoadFile(path, 1);
}

byte *COM_LoadHunkFile2 (const char *path, unsigned int *path_id)
{
        return COM_LoadFile2 (path, LOADFILE_HUNK, path_id);
}

void COM_LoadCacheFile(char *path, struct cache_user_s *cu)
{
	loadcache = cu;
	COM_LoadFile(path, 3);
}


unsigned char *COM_LoadStackFile(char *path, void *buffer, int bufsize)
{ // uses temp hunk if larger than bufsize
	loadbuf = (unsigned char *) buffer;
	loadsize = bufsize;
	return COM_LoadFile(path, 4);
}

pack_t *COM_LoadPackFile(char *packfile)
{	// johnfitz -- modified based on topaz's tutorial
	// Takes an explicit (not game tree related) path to a pak file.
	// Loads the header and directory, adding the files at the beginning
	// of the list so they override previous pack files.
	int packhandle;
	if (Sys_FileOpenRead(packfile, &packhandle) == -1)
		return NULL;
	dpackheader_t header;
	Sys_FileRead(packhandle, (void *)&header, sizeof(header));
	if (header.id[0] != 'P' || header.id[1] != 'A' || header.id[2] != 'C'
	    || header.id[3] != 'K')
		Sys_Error("%s is not a packfile", packfile);
	header.dirofs = LittleLong(header.dirofs);
	header.dirlen = LittleLong(header.dirlen);
	int numpackfiles = header.dirlen / sizeof(dpackfile_t);
	if (numpackfiles > MAX_FILES_IN_PACK)
		Sys_Error("%s has %i files", packfile, numpackfiles);
	if (numpackfiles != PAK0_COUNT)
		com_modified = true;	// not the original file
	//johnfitz -- dynamic gamedir loading
	packfile_t *newfiles = Z_Malloc(numpackfiles * sizeof(packfile_t));
	Sys_FileSeek(packhandle, header.dirofs);
	dpackfile_t info[MAX_FILES_IN_PACK];
	Sys_FileRead(packhandle, (void *)info, header.dirlen);
	// crc the directory to check for modifications
	unsigned short crc;
	CRC_Init(&crc);
	for (int i = 0; i < header.dirlen; i++)
		CRC_ProcessByte(&crc, ((byte *) info)[i]);
	if (crc != PAK0_CRC)
		com_modified = true;
	// parse the directory
	for (int i = 0; i < numpackfiles; i++) {
		strcpy(newfiles[i].name, info[i].name);
		newfiles[i].filepos = LittleLong(info[i].filepos);
		newfiles[i].filelen = LittleLong(info[i].filelen);
	}
	// johnfitz -- dynamic gamedir loading
	pack_t *pack = Z_Malloc(sizeof(pack_t));
	strcpy(pack->filename, packfile);
	pack->handle = packhandle;
	pack->numfiles = numpackfiles;
	pack->files = newfiles;
	return pack;
}

void COM_AddGameDirectory(char *dir)
{ // johnfitz -- modified based on topaz's tutorial
	char pakfile[MAX_OSPATH];
	strcpy(com_gamedir, dir);
	// add the directory to the search path
	searchpath_t *search = Z_Malloc(sizeof(searchpath_t));
	strcpy(search->filename, dir);
	search->next = com_searchpaths;
	com_searchpaths = search;
	// add any pak files in the format pak0.pak pak1.pak, ...
	for (int i = 0; ; i++) {
		sprintf(pakfile, "%s/pak%i.pak", dir, i);
		pack_t *pak = COM_LoadPackFile(pakfile);
		if (!pak) { // CyanBun96: capitalize the path and try again
			for (int j = 0; pakfile[j]; j++)
				if (pakfile[j] >= 'a' && pakfile[j] <= 'z')
					pakfile[j] &= 0b11011111;
			pak = COM_LoadPackFile(pakfile);
			if (!pak) break;
		}
		search = Z_Malloc(sizeof(searchpath_t));
		search->pack = pak;
		search->next = com_searchpaths;
		com_searchpaths = search;
	}
}

void COM_InitFilesystem() //johnfitz -- modified based on topaz's tutorial
{
	char basedir[MAX_OSPATH];
	int i = COM_CheckParm("-basedir");
	if (i && i < com_argc - 1)
		strcpy(basedir, com_argv[i + 1]);
	else
		strcpy(basedir, host_parms.basedir);
	int j = strlen(basedir);
	if (j > 0 && ((basedir[j - 1] == '\\') || (basedir[j - 1] == '/')))
		basedir[j - 1] = 0;
	i = COM_CheckParm("-cachedir");
	if (i && i < com_argc - 1) {
		if (com_argv[i + 1][0] == '-')
			com_cachedir[0] = 0;
		else
			strcpy(com_cachedir, com_argv[i + 1]);
	} else if (host_parms.cachedir)
		strcpy(com_cachedir, host_parms.cachedir);
	else
		com_cachedir[0] = 0;
	// start up with GAMENAME by default (id1)
	COM_AddGameDirectory(va("%s/" GAMENAME, basedir));
	strcpy(com_gamedir, va("%s/" GAMENAME, basedir));
	// johnfitz -- track number of mission packs added
	// since we don't want to allow the "game" command to strip them away
	com_nummissionpacks = 0;
	if (COM_CheckParm("-rogue")) {
		COM_AddGameDirectory(va("%s/rogue", basedir));
		com_nummissionpacks++;
	}
	if (COM_CheckParm("-hipnotic")) {
		COM_AddGameDirectory(va("%s/hipnotic", basedir));
		com_nummissionpacks++;
	}
	if (COM_CheckParm("-quoth")) {
		COM_AddGameDirectory(va("%s/quoth", basedir));
		com_nummissionpacks++;
	} //johnfitz
	if ((i = COM_CheckParm("-game")) && i < com_argc - 1) {
		com_modified = true;
		COM_AddGameDirectory(va("%s/%s", basedir, com_argv[i + 1]));
	}
	if ((i = COM_CheckParm("-path"))) {
		com_modified = true;
		com_searchpaths = NULL;
		while (++i < com_argc) {
			if (!com_argv[i] || com_argv[i][0] == '+'
			    || com_argv[i][0] == '-')
				break;
			searchpath_t *search = Hunk_Alloc(sizeof(searchpath_t));
			if (!strcmp(COM_FileExtension(com_argv[i]), "pak")) {
				search->pack = COM_LoadPackFile(com_argv[i]);
				if (!search->pack)
					Sys_Error("Couldn't load packfile: %s",
						  com_argv[i]);
			} else
				strcpy(search->filename, com_argv[i]);
			search->next = com_searchpaths;
			com_searchpaths = search;
		}
	}
	if (COM_CheckParm("-proghack"))
		proghack = true;
}

void COM_AddExtension (char *path, const char *extension, size_t len)
{
        if (strcmp(COM_FileGetExtension(path), extension + 1) != 0)
                strlcat(path, extension, len);
}

/*
================
LOC_Format

Replaces placeholders (of the form {} or {N}) with the corresponding arguments

Returns number of written chars, excluding the NUL terminator
If len > 0, output is always NUL-terminated
================
*/
size_t LOC_Format (const char *format, const char* (*getarg_fn) (int idx, void* userdata), void* userdata, char* out, size_t len)
{
        size_t written = 0;
        int numargs = 0;

        if (!len)
        {
                Con_DPrintf("LOC_Format: no output space\n");
                return 0;
        }
        --len; // reserve space for the terminator

        while (*format && written < len)
        {
                const char* insert;
                size_t space_left;
                size_t insert_len;
                int argindex = LOC_ParseArg(&format);

                if (argindex < 0)
                {
                        out[written++] = *format++;
                        continue;
                }

                insert = getarg_fn(argindex, userdata);
                space_left = len - written;
                insert_len = Q_strlen(insert);

                if (insert_len > space_left)
                {
                        Con_DPrintf("LOC_Format: overflow at argument #%d\n", numargs);
                        insert_len = space_left;
                }

                Q_memcpy(out + written, insert, insert_len);
                written += insert_len;
        }

        if (*format)
                Con_DPrintf("LOC_Format: overflow\n");

        out[written] = 0;

        return written;
}
