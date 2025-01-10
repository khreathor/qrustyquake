// Copyright (C) 1996-2001 Id Software, Inc.
// Copyright (C) 2010-2014 QuakeSpasm developers
// GPLv3 See LICENSE for details.

#ifndef __NET_DATAGRAM_H
#define __NET_DATAGRAM_H
int Datagram_Init ();
void Datagram_Listen (qboolean state);
void Datagram_SearchForHosts (qboolean xmit);
qsocket_t *Datagram_Connect (const char *host);
qsocket_t *Datagram_CheckNewConnections ();
int Datagram_GetMessage (qsocket_t *sock);
int Datagram_SendMessage (qsocket_t *sock, sizebuf_t *data);
int Datagram_SendUnreliableMessage (qsocket_t *sock, sizebuf_t *data);
qboolean Datagram_CanSendMessage (qsocket_t *sock);
qboolean Datagram_CanSendUnreliableMessage (qsocket_t *sock);
void Datagram_Close (qsocket_t *sock);
void Datagram_Shutdown ();
#endif
