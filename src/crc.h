// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

void CRC_Init(unsigned short *crcvalue);
void CRC_ProcessByte(unsigned short *crcvalue, byte data);
unsigned short CRC_Value(unsigned short crcvalue);
