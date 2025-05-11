// Copyright (C) 1996-2001 Id Software, Inc.
// Copyright (C) 2002-2009 John Fitzgibbons and others
// Copyright (C) 2010-2014 QuakeSpasm developers
// GPLv3 See LICENSE for details.

#ifndef QUAKE_SERVER_H
#define QUAKE_SERVER_H

// server.h

typedef struct
{
	int			maxclients;
	int			maxclientslimit;
	struct client_s	*clients;		// [maxclients]
	int			serverflags;		// episode completion information
	qboolean	changelevel_issued;	// cleared when at SV_SpawnServer
} server_static_t;

//=============================================================================


typedef enum {ss_loading, ss_active} server_state_t;

typedef struct
{
	qboolean	active;				// false if only a net client

	qboolean	paused;
	qboolean	loadgame;			// handle connections specially
	qboolean	nomonsters;			// server started with 'nomonsters' cvar active

	double		time;

	int			lastcheck;			// used by PF_checkclient
	double		lastchecktime;

	char		name[64];			// map name
	char		modelname[64];		// maps/<name>.bsp, for model_precache[0]
	struct model_s	*worldmodel;
	const char	*model_precache[MAX_MODELS];	// NULL terminated
	struct model_s	*models[MAX_MODELS];
	const char	*sound_precache[MAX_SOUNDS];	// NULL terminated
	const char	*lightstyles[MAX_LIGHTSTYLES];
	int			num_edicts;
	int			max_edicts;
	edict_t		*edicts;			// can NOT be array indexed, because
									// edict_t is variable sized, but can
									// be used to reference the world ent
	server_state_t	state;			// some actions are only valid during load

	sizebuf_t	datagram;
	byte		datagram_buf[MAX_DATAGRAM];

	sizebuf_t	reliable_datagram;	// copied to all clients at end of frame
	byte		reliable_datagram_buf[MAX_DATAGRAM];

	sizebuf_t	*signon;
	int			num_signon_buffers;
	sizebuf_t	*signon_buffers[MAX_SIGNON_BUFFERS];

	unsigned	protocol; //johnfitz
	unsigned	protocolflags;
} server_t;



enum sendsignon_e
{
	PRESPAWN_DONE,
	PRESPAWN_FLUSH=1,
	PRESPAWN_SIGNONBUFS,
	PRESPAWN_SIGNONMSG,
};

typedef struct client_s
{
        qboolean                active;                         // false = client is free
        qboolean                spawned;                        // false = don't send datagrams
        qboolean                dropasap;                       // has been told to go to another level
        enum sendsignon_e       sendsignon;                     // only valid before spawned
        int                             signonidx;

        double                  last_message;           // reliable messages must be sent
                                                                                // periodically

        struct qsocket_s *netconnection;        // communications handle

        usercmd_t               cmd;                            // movement
        vec3_t                  wishdir;                        // intended motion calced from cmd

        sizebuf_t               message;                        // can be added to at any time,
                                                                                // copied and clear once per frame
        byte                    msgbuf[MAX_MSGLEN];
        edict_t                 *edict;                         // EDICT_NUM(clientnum+1)
        char                    name[32];                       // for printing to other people
        int                             colors;

        float                   ping_times[NUM_PING_TIMES];
        int                             num_pings;                      // ping_times[num_pings%NUM_PING_TIMES]

// spawn parms are carried from level to level
        float                   spawn_parms[NUM_SPAWN_PARMS];

// client known data for deltas
        int                             old_frags;
} client_t;

//=============================================================================



//============================================================================

extern	server_static_t	svs;				// persistant server info
extern	server_t		sv;					// local server

extern	client_t	*host_client;

extern	edict_t		*sv_player;

//===========================================================

void SV_Init (void);

void SV_StartParticle (vec3_t org, vec3_t dir, int color, int count);
void SV_StartSound (edict_t *entity, int channel, const char *sample, int volume,
    float attenuation);
void SV_LocalSound (client_t *client, const char *sample); // for 2021 rerelease

void SV_DropClient (qboolean crash);

void SV_SendClientMessages (void);
void SV_ClearDatagram (void);
void SV_ReserveSignonSpace (int numbytes);

int SV_ModelIndex (const char *name);

void SV_SetIdealPitch (void);

void SV_AddUpdates (void);

void SV_ClientThink (void);
void SV_AddClientToServer (struct qsocket_s	*ret);

void SV_ClientPrintf (const char *fmt, ...);// FUNC_PRINTF(1,2);
void SV_BroadcastPrintf (const char *fmt, ...);// FUNC_PRINTF(1,2);

void SV_Physics (void);

qboolean SV_CheckBottom (edict_t *ent);
qboolean SV_movestep (edict_t *ent, vec3_t move, qboolean relink);

void SV_WriteClientdataToMessage (edict_t *ent, sizebuf_t *msg);

void SV_MoveToGoal (void);

void SV_CheckForNewClients (void);
void SV_RunClients (void);
void SV_SaveSpawnparms (void);
void SV_SpawnServer (const char *server);

#endif	/* QUAKE_SERVER_H */
