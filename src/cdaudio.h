// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.


int CDAudio_Init(void);
void CDAudio_Play(byte track, qboolean looping);
void CDAudio_Stop(void);
void CDAudio_Pause(void);
void CDAudio_Resume(void);
void CDAudio_Shutdown(void);
void CDAudio_Update(void);
