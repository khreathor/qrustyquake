// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.


#ifndef __net_udp_h
#define __net_udp_h

sys_socket_t  UDP_Init (void);
void UDP_Shutdown (void);
void UDP_Listen (qboolean state);
sys_socket_t  UDP_OpenSocket (int port);
int  UDP_CloseSocket (sys_socket_t socketid);
int  UDP_Connect (sys_socket_t socketid, struct qsockaddr *addr);
sys_socket_t  UDP_CheckNewConnections (void);
int  UDP_Read (sys_socket_t socketid, byte *buf, int len, struct qsockaddr *addr);
int  UDP_Write (sys_socket_t socketid, byte *buf, int len, struct qsockaddr *addr);
int  UDP_Broadcast (sys_socket_t socketid, byte *buf, int len);
const char *UDP_AddrToString (struct qsockaddr *addr);
int  UDP_StringToAddr (const char *string, struct qsockaddr *addr);
int  UDP_GetSocketAddr (sys_socket_t socketid, struct qsockaddr *addr);
int  UDP_GetNameFromAddr (struct qsockaddr *addr, char *name);
int  UDP_GetAddrFromName (const char *name, struct qsockaddr *addr);
int  UDP_AddrCompare (struct qsockaddr *addr1, struct qsockaddr *addr2);
int  UDP_GetSocketPort (struct qsockaddr *addr);
int  UDP_SetSocketPort (struct qsockaddr *addr, int port);

#endif	/* __net_udp_h */

