// Copyright (C) 1996-2001 Id Software, Inc.
// Copyright (C) 2002-2009 John Fitzgibbons and others
// Copyright (C) 2009-2010 Ozkan Sezer
// Copyright (C) 2010-2014 QuakeSpasm developers
// GPLv3 See LICENSE for details.

// net.h: quake's interface to the networking layer network functions and data
// common to the whole engine

extern int DEFAULTnet_hostport;
extern int net_hostport;
extern double net_time;
extern sizebuf_t net_message;
extern int net_activeconnections;
// Server list related globals:
extern bool slistInProgress;
extern bool slistSilent;
extern bool slistLocal;
extern int hostCacheCount;
// FIXME: driver related, but public:
extern int ipxAvailable;
extern bool tcpipAvailable;
extern char my_ipx_address[NET_NAMELEN];
extern char my_tcpip_address[NET_NAMELEN];

void NET_Init();
void NET_Shutdown();
struct qsocket_s *NET_CheckNewConnections(); // returns a new connection number
					// if there is one pending, else -1
struct qsocket_s *NET_Connect(const char *host); // called by client to connect
					// to a host. Returns -1 if not able to
bool NET_CanSendMessage(struct qsocket_s *sock); // Returns true or false if
	// the given qsocket can currently accept a message to be transmitted.
int NET_GetMessage(struct qsocket_s *sock);
// returns data in net_message sizebuf
// returns 0 if no data is waiting
// returns 1 if a message was received
// returns 2 if an unreliable message was received
// returns -1 if the connection died

int NET_SendMessage(struct qsocket_s *sock, sizebuf_t *data);
int NET_SendUnreliableMessage(struct qsocket_s *sock, sizebuf_t *data);
// returns 0 if the message connot be delivered reliably, but the connection
// is still considered valid
// returns 1 if the message was sent properly
// returns -1 if the connection died

int NET_SendToAll(sizebuf_t *data, double blocktime); // This is a reliable
				// *blocking* send to all attached clients.

void NET_Close(struct qsocket_s *sock);
// if a dead connection is returned by a get or send function, this function
// should be called when it is convenient

// Server calls when a client is kicked off for a game related misbehavior
// like an illegal protocal conversation. Client calls when disconnecting
// from a server.
// A netcon_t number will not be reused until this function is called for it

void NET_Poll();
void NET_Slist_f();
