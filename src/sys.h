// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.


#define	MAX_HANDLES 32

// File IO
int Sys_FileType (const char *path);
int Sys_FileOpenRead (char *path, int *hndl);
int Sys_FileOpenWrite (char *path);
void Sys_FileClose (int handle);
void Sys_FileSeek (int handle, int position);
int Sys_FileRead (int handle, void *dest, int count);
int Sys_FileWrite (int handle, void *data, int count);
int Sys_FileTime (char *path);
void Sys_mkdir (char *path);

// System IO
void Sys_Error (char *error, ...);
void Sys_Printf (char *fmt, ...);
void Sys_Quit ();
double Sys_FloatTime ();
double Sys_DoubleTime ();
