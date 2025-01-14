// Copyright (C) 2007-2012 O.Sezer <sezero@users.sourceforge.net>
// GPLv3 See LICENSE for details.

// net_sys.h -- common network system header.

#include <sys/types.h>
#include <errno.h>
#include <stddef.h>
#include <limits.h>

#undef HAVE_SA_LEN
#define SA_FAM_OFFSET 0

#ifndef _WIN32
#include <sys/param.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define SOCKETERRNO errno
#define ioctlsocket ioctl
#define closesocket close
#define selectsocket select
#define IOCTLARG_P(x) /* (char *) */ x
#define NET_EWOULDBLOCK EWOULDBLOCK
#define NET_ECONNREFUSED ECONNREFUSED
#define socketerror(x) strerror((x))
typedef int sys_socket_t;
#endif

#ifdef _WIN32
#include <winsock2.h> // includes windows.h
#include <ws2tcpip.h>
#define MAXHOSTNAMELEN 1024
#define selectsocket select
#define IOCTLARG_P(x) /* (u_long *) */ x
#define SOCKETERRNO WSAGetLastError()
#define NET_EWOULDBLOCK WSAEWOULDBLOCK
#define NET_ECONNREFUSED WSAECONNREFUSED
// must #include "wsaerror.h" for this :
#define socketerror(x) __WSAE_StrError((x))
// there is no in_addr_t on windows: define it as
// the type of the S_addr of in_addr structure 
typedef u_long in_addr_t;
typedef SOCKET sys_socket_t;
#endif
