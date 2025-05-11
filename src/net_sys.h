// Copyright (C) 2007-2012 O.Sezer <sezero@users.sourceforge.net>
// GPLv3 See LICENSE for details.

// net_sys.h -- common network system header.

#include <sys/types.h>
#include <errno.h>
#include <stddef.h>
#include <limits.h>

#ifndef _WIN32
#include <sys/param.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
typedef int sys_socket_t;
#else
#include <winsock2.h> // includes windows.h
#include <ws2tcpip.h>
// there is no in_addr_t on windows: define it as
// the type of the S_addr of in_addr structure 
typedef u_long in_addr_t;
typedef SOCKET sys_socket_t;
#endif
