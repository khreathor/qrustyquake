// Copyright (C) 1996-2001 Id Software, Inc.
// Copyright (C) 2010-2014 QuakeSpasm developers
// GPLv3 See LICENSE for details.

#ifndef __NET_WINIPX_H
#define __NET_WINIPX_H

sys_socket_t  WIPX_Init ();
void WIPX_Shutdown ();
void WIPX_Listen (qboolean state);
sys_socket_t  WIPX_OpenSocket (int port);
int WIPX_CloseSocket (sys_socket_t socketid);
int WIPX_Connect (sys_socket_t socketid, struct qsockaddr *addr);
sys_socket_t  WIPX_CheckNewConnections ();
int WIPX_Read (sys_socket_t socketid, byte *buf, int len, struct qsockaddr *addr);
int WIPX_Write (sys_socket_t socketid, byte *buf, int len, struct qsockaddr *addr);
int WIPX_Broadcast (sys_socket_t socketid, byte *buf, int len);
const char *WIPX_AddrToString (struct qsockaddr *addr);
int WIPX_StringToAddr (const char *string, struct qsockaddr *addr);
int WIPX_GetSocketAddr (sys_socket_t socketid, struct qsockaddr *addr);
int WIPX_GetNameFromAddr (struct qsockaddr *addr, char *name);
int WIPX_GetAddrFromName (const char *name, struct qsockaddr *addr);
int WIPX_AddrCompare (struct qsockaddr *addr1, struct qsockaddr *addr2);
int WIPX_GetSocketPort (struct qsockaddr *addr);
int WIPX_SetSocketPort (struct qsockaddr *addr, int port);

#endif
