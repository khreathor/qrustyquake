#include "sys_sdl.c"
s32 Sys_FileOpenWrite (const s8 *path) // CyanBun96: pasta from Quakespasm
{
	s32 i = findhandle ();
	FILE *f = fopen(path, "wb");
	if(!f) Sys_Error ("Error opening %s: %s", path, strerror(errno));
	sys_handles[i] = f;
	return i;
}

s32 Sys_FileWrite (s32 handle, const void *data, s32 count) // ditto
{
	return fwrite (data, 1, count, sys_handles[handle]);
}

void Sys_mkdir(const s8 *path)
{
	_mkdir(path);
}

f64 Sys_FloatTime()
{
	static s32 starttime = 0;
	if(!starttime)
		starttime = clock();
	return (clock() - starttime) * 1.0 / 1024;
}

f64 Sys_DoubleTime(){ return Sys_FloatTime(); }

#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#endif
s32 Sys_FileType (const s8 *path)
{
        DWORD result = GetFileAttributes(path);

        if(result == INVALID_FILE_ATTRIBUTES)
                return FS_ENT_NONE;
        if(result & FILE_ATTRIBUTE_DIRECTORY)
                return FS_ENT_DIRECTORY;

        return FS_ENT_FILE;
}
