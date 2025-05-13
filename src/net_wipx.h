// Copyright (C) 1996-2001 Id Software, Inc.
// Copyright (C) 2010-2014 QuakeSpasm developers
// GPLv3 See LICENSE for details.

sys_socket_t WIPX_Init();
void WIPX_Shutdown();
void WIPX_Listen(bool state);
sys_socket_t WIPX_OpenSocket(s32 port);
s32 WIPX_CloseSocket(sys_socket_t socketid);
s32 WIPX_Connect(sys_socket_t socketid, struct qsockaddr *addr);
sys_socket_t WIPX_CheckNewConnections();
s32 WIPX_Read(sys_socket_t socketid, u8 *buf, s32 len, struct qsockaddr *addr);
s32 WIPX_Write(sys_socket_t socketid, u8 *buf, s32 len, struct qsockaddr *addr);
s32 WIPX_Broadcast(sys_socket_t socketid, u8 *buf, s32 len);
const s8 *WIPX_AddrToString(struct qsockaddr *addr);
s32 WIPX_StringToAddr(const s8 *string, struct qsockaddr *addr);
s32 WIPX_GetSocketAddr(sys_socket_t socketid, struct qsockaddr *addr);
s32 WIPX_GetNameFromAddr(struct qsockaddr *addr, s8 *name);
s32 WIPX_GetAddrFromName(const s8 *name, struct qsockaddr *addr);
s32 WIPX_AddrCompare(struct qsockaddr *addr1, struct qsockaddr *addr2);
s32 WIPX_GetSocketPort(struct qsockaddr *addr);
s32 WIPX_SetSocketPort(struct qsockaddr *addr, s32 port);
