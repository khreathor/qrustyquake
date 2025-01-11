// Copyright (C) 1996-2001 Id Software, Inc.
// Copyright (C) 2010-2014 QuakeSpasm developers
// GPLv3 See LICENSE for details.

sys_socket_t WINS_Init();
void WINS_Shutdown();
void WINS_Listen(qboolean state);
sys_socket_t WINS_OpenSocket(int port);
int WINS_CloseSocket(sys_socket_t socketid);
int WINS_Connect(sys_socket_t socketid, struct qsockaddr *addr);
sys_socket_t WINS_CheckNewConnections();
int WINS_Read(sys_socket_t socketid, byte *buf, int len, struct qsockaddr *addr);
int WINS_Write(sys_socket_t socketid, byte *buf, int len, struct qsockaddr *addr);
int WINS_Broadcast(sys_socket_t socketid, byte *buf, int len);
const char *WINS_AddrToString(struct qsockaddr *addr);
int WINS_StringToAddr(const char *string, struct qsockaddr *addr);
int WINS_GetSocketAddr(sys_socket_t socketid, struct qsockaddr *addr);
int WINS_GetNameFromAddr(struct qsockaddr *addr, char *name);
int WINS_GetAddrFromName(const char *name, struct qsockaddr *addr);
int WINS_AddrCompare(struct qsockaddr *addr1, struct qsockaddr *addr2);
int WINS_GetSocketPort(struct qsockaddr *addr);
int WINS_SetSocketPort(struct qsockaddr *addr, int port);
