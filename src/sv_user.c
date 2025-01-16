// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

// sv_user.c -- server code for moving users

#include "quakedef.h"

#define	MAX_FORWARD	6

static vec3_t forward, right, up;
edict_t *sv_player;
vec3_t wishdir;
float wishspeed;
float *angles; // world
float *origin;
float *velocity;
qboolean onground;
usercmd_t cmd;

extern cvar_t sv_friction;
extern cvar_t sv_stopspeed;

cvar_t sv_edgefriction = { "edgefriction", "2", false, false, 0, NULL };
cvar_t sv_idealpitchscale = { "sv_idealpitchscale", "0.8", false, false,0,NULL};
cvar_t sv_maxspeed = { "sv_maxspeed", "320", false, true, 0, NULL };
cvar_t sv_accelerate = { "sv_accelerate", "10", false, false, 0, NULL };

void SV_SetIdealPitch()
{
	if (!((int)sv_player->v.flags & FL_ONGROUND))
		return;
	float angleval = sv_player->v.angles[YAW] * M_PI * 2 / 360;
	float sinval = sin(angleval);
	float cosval = cos(angleval);
	float z[MAX_FORWARD];
	int i = 0;
	for (; i < MAX_FORWARD; i++) {
		vec3_t top, bottom;
		top[0] = sv_player->v.origin[0] + cosval * (i + 3) * 12;
		top[1] = sv_player->v.origin[1] + sinval * (i + 3) * 12;
		top[2] = sv_player->v.origin[2] + sv_player->v.view_ofs[2];
		bottom[0] = top[0];
		bottom[1] = top[1];
		bottom[2] = top[2] - 160;
		trace_t tr = SV_Move(top, vec3_origin, vec3_origin, bottom,
				1, sv_player);
		if (tr.allsolid // looking at a wall, leave ideal the way is was
			|| tr.fraction == 1) // near a dropoff
			return;
		z[i] = top[2] + tr.fraction * (bottom[2] - top[2]);
	}
	int dir = 0;
	int steps = 0;
	for (int j = 1; j < i; j++) {
		int step = z[j] - z[j - 1];
		if (step > -ON_EPSILON && step < ON_EPSILON)
			continue;
		if (dir && (step-dir > ON_EPSILON || step - dir < -ON_EPSILON))
			return; // mixed changes
		steps++;
		dir = step;
	}
	if (!dir) {
		sv_player->v.idealpitch = 0;
		return;
	}
	if (steps < 2)
		return;
	sv_player->v.idealpitch = -dir * sv_idealpitchscale.value;
}

void SV_UserFriction()
{
	float *vel = velocity;
	float speed = sqrt(vel[0] * vel[0] + vel[1] * vel[1]);
	if (!speed)
		return;
	// if the leading edge is over a dropoff, increase friction
	vec3_t start, stop;
	start[0] = stop[0] = origin[0] + vel[0] / speed * 16;
	start[1] = stop[1] = origin[1] + vel[1] / speed * 16;
	start[2] = origin[2] + sv_player->v.mins[2];
	stop[2] = start[2] - 34;
	trace_t trace =
		SV_Move(start, vec3_origin, vec3_origin, stop, true, sv_player);
	float friction = sv_friction.value *
		(trace.fraction == 1.0 ? sv_edgefriction.value : 1);
	float control = speed < sv_stopspeed.value ? sv_stopspeed.value : speed;
	float newspeed = speed - host_frametime * control * friction;
	if (newspeed < 0)
		newspeed = 0;
	newspeed /= speed;
	vel[0] = vel[0] * newspeed;
	vel[1] = vel[1] * newspeed;
	vel[2] = vel[2] * newspeed;
}

void SV_Accelerate()
{
	int i;
	float addspeed, accelspeed, currentspeed;

	currentspeed = DotProduct(velocity, wishdir);
	addspeed = wishspeed - currentspeed;
	if (addspeed <= 0)
		return;
	accelspeed = sv_accelerate.value * host_frametime * wishspeed;
	if (accelspeed > addspeed)
		accelspeed = addspeed;

	for (i = 0; i < 3; i++)
		velocity[i] += accelspeed * wishdir[i];
}

void SV_AirAccelerate(vec3_t wishveloc)
{
	float wishspd = VectorNormalize(wishveloc);
	if (wishspd > 30)
		wishspd = 30;
	float currentspeed = DotProduct(velocity, wishveloc);
	float addspeed = wishspd - currentspeed;
	if (addspeed <= 0)
		return;
	float accelspeed = sv_accelerate.value * wishspeed * host_frametime;
	if (accelspeed > addspeed)
		accelspeed = addspeed;
	for (int i = 0; i < 3; i++)
		velocity[i] += accelspeed * wishveloc[i];
}

void DropPunchAngle()
{
	float len = VectorNormalize(sv_player->v.punchangle);
	len -= 10 * host_frametime;
	if (len < 0)
		len = 0;
	VectorScale(sv_player->v.punchangle, len, sv_player->v.punchangle);
}

void SV_WaterMove()
{
	AngleVectors(sv_player->v.v_angle, forward, right, up);
	vec3_t wishvel;
	for (int i = 0; i < 3; i++) // user intentions
		wishvel[i] =
			forward[i] * cmd.forwardmove + right[i] * cmd.sidemove;
	if (!cmd.forwardmove && !cmd.sidemove && !cmd.upmove)
		wishvel[2] -= 60; // drift towards bottom
	else
		wishvel[2] += cmd.upmove;
	float wishspeed = Length(wishvel);
	if (wishspeed > sv_maxspeed.value) {
		VectorScale(wishvel, sv_maxspeed.value / wishspeed, wishvel);
		wishspeed = sv_maxspeed.value;
	}
	wishspeed *= 0.7;
	float speed = Length(velocity); // water friction
	float newspeed = 0;
	if (speed) {
		newspeed = speed - host_frametime * speed * sv_friction.value;
		if (newspeed < 0)
			newspeed = 0;
		VectorScale(velocity, newspeed / speed, velocity);
	}
	if (!wishspeed) // water acceleration
		return;
	float addspeed = wishspeed - newspeed;
	if (addspeed <= 0)
		return;
	VectorNormalize(wishvel);
	float accelspeed = sv_accelerate.value * wishspeed * host_frametime;
	if (accelspeed > addspeed)
		accelspeed = addspeed;
	for (int i = 0; i < 3; i++)
		velocity[i] += accelspeed * wishvel[i];
}

void SV_WaterJump()
{
	if (sv.time > sv_player->v.teleport_time || !sv_player->v.waterlevel) {
		sv_player->v.flags = (int)sv_player->v.flags & ~FL_WATERJUMP;
		sv_player->v.teleport_time = 0;
	}
	sv_player->v.velocity[0] = sv_player->v.movedir[0];
	sv_player->v.velocity[1] = sv_player->v.movedir[1];
}

void SV_AirMove()
{
	AngleVectors(sv_player->v.angles, forward, right, up);
	float fmove = cmd.forwardmove;
	float smove = cmd.sidemove;
	if (sv.time < sv_player->v.teleport_time && fmove < 0)
		fmove = 0; // hack to not let you back into teleporter
	vec3_t wishvel;
	for (int i = 0; i < 3; i++)
		wishvel[i] = forward[i] * fmove + right[i] * smove;
	wishvel[2] = 0;
	if ((int)sv_player->v.movetype != MOVETYPE_WALK)
		wishvel[2] = cmd.upmove;
	VectorCopy(wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);
	if (wishspeed > sv_maxspeed.value) {
		VectorScale(wishvel, sv_maxspeed.value / wishspeed, wishvel);
		wishspeed = sv_maxspeed.value;
	}
	if (sv_player->v.movetype == MOVETYPE_NOCLIP) { // noclip
		VectorCopy(wishvel, velocity);
	} else if (onground) {
		SV_UserFriction();
		SV_Accelerate();
	} else // not on ground, so little effect on velocity
		SV_AirAccelerate(wishvel);
}



void SV_ClientThink() // the move fields specify an intended velocity in pix/sec
{ // the angle fields specify an exact angular motion in degrees
	if (sv_player->v.movetype == MOVETYPE_NONE)
		return;
	onground = (int)sv_player->v.flags & FL_ONGROUND;
	origin = sv_player->v.origin;
	velocity = sv_player->v.velocity;
	DropPunchAngle();
	if (sv_player->v.health <= 0)
		return; // if dead, behave differently
	// show 1/3 the pitch angle and all the roll angle
	cmd = host_client->cmd;
	angles = sv_player->v.angles;
	vec3_t v_angle;
	VectorAdd(sv_player->v.v_angle, sv_player->v.punchangle, v_angle);
	angles[ROLL] = V_CalcRoll(sv_player->v.angles, sv_player->v.velocity)*4;
	if (!sv_player->v.fixangle) {
		angles[PITCH] = -v_angle[PITCH] / 3;
		angles[YAW] = v_angle[YAW];
	}
	if ((int)sv_player->v.flags & FL_WATERJUMP) {
		SV_WaterJump();
		return;
	}
	if ((sv_player->v.waterlevel >= 2) // walk
	    && (sv_player->v.movetype != MOVETYPE_NOCLIP)) {
		SV_WaterMove();
		return;
	}
	SV_AirMove();
}

void SV_ReadClientMove(usercmd_t *move)
{
	// read ping time
	host_client->ping_times[host_client->num_pings % NUM_PING_TIMES]
	    = sv.time - MSG_ReadFloat();
	host_client->num_pings++;
	// read current angles  
	vec3_t angle;
	for (int i = 0; i < 3; i++)
		angle[i] = MSG_ReadAngle();
	VectorCopy(angle, host_client->edict->v.v_angle);
	// read movement
	move->forwardmove = MSG_ReadShort();
	move->sidemove = MSG_ReadShort();
	move->upmove = MSG_ReadShort();
	// read buttons
	int bits = MSG_ReadByte();
	host_client->edict->v.button0 = bits & 1;
	host_client->edict->v.button2 = (bits & 2) >> 1;
	int i = MSG_ReadByte();
	if (i)
		host_client->edict->v.impulse = i;
}

qboolean SV_ReadClientMessage() // Returns false if the client should be killed
{
	int ret;
	do {
nextmsg:
		ret = NET_GetMessage(host_client->netconnection);
		if (ret == -1) {
			Sys_Printf
			    ("SV_ReadClientMessage: NET_GetMessage failed\n");
			return false;
		}
		if (!ret)
			return true;
		MSG_BeginReading();
		while (1) {
			if (!host_client->active)
				return false; // a command caused an error
			if (msg_badread) {
				Sys_Printf("SV_ReadClientMessage: badread\n");
				return false;
			}
			int cmd = MSG_ReadChar();
			switch (cmd) {
			case -1:
				goto nextmsg; // end of message
			default:
				Sys_Printf
				    ("SV_ReadClientMessage: unknown command char\n");
				return false;
			case clc_nop:
				break;
			case clc_stringcmd:
				char *s = MSG_ReadString();
				if (host_client->privileged)
					ret = 2;
				else
					ret = 0;
				if (Q_strncasecmp(s, "status", 6) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "god", 3) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "notarget", 8) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "fly", 3) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "name", 4) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "noclip", 6) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "say", 3) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "say_team", 8) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "tell", 4) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "color", 5) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "kill", 4) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "pause", 5) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "spawn", 5) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "begin", 5) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "prespawn", 8) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "kick", 4) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "ping", 4) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "give", 4) == 0)
					ret = 1;
				else if (Q_strncasecmp(s, "ban", 3) == 0)
					ret = 1;
				if (ret == 2)
					Cbuf_InsertText(s);
				else if (ret == 1)
					Cmd_ExecuteString(s, src_client);
				else
					Con_DPrintf("%s tried to %s\n",
						    host_client->name, s);
				break;
			case clc_disconnect:
				return false;
			case clc_move:
				SV_ReadClientMove(&host_client->cmd);
				break;
			}
		}
	} while (ret == 1);
	return true;
}

void SV_RunClients()
{
	int i = 0;
	for (host_client = svs.clients; i < svs.maxclients; i++, host_client++){
		if (!host_client->active)
			continue;
		sv_player = host_client->edict;
		if (!SV_ReadClientMessage()) {
			SV_DropClient(false);	// client misbehaved...
			continue;
		}
		if (!host_client->spawned) {
			// clear client movement until a new packet is received
			memset(&host_client->cmd, 0, sizeof(host_client->cmd));
			continue;
		}
		// always pause in single player if in console or menus
		if (!sv.paused && (svs.maxclients > 1 || key_dest == key_game))
			SV_ClientThink();
	}
}
