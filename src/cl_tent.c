// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

// cl_tent.c -- client side temporary entities

#include "quakedef.h"

int num_temp_entities;
entity_t cl_temp_entities[MAX_TEMP_ENTITIES];
beam_t cl_beams[MAX_BEAMS];

sfx_t *cl_sfx_wizhit;
sfx_t *cl_sfx_knighthit;
sfx_t *cl_sfx_tink1;
sfx_t *cl_sfx_ric1;
sfx_t *cl_sfx_ric2;
sfx_t *cl_sfx_ric3;
sfx_t *cl_sfx_r_exp3;

void CL_InitTEnts()
{
	cl_sfx_wizhit = S_PrecacheSound("wizard/hit.wav");
	cl_sfx_knighthit = S_PrecacheSound("hknight/hit.wav");
	cl_sfx_tink1 = S_PrecacheSound("weapons/tink1.wav");
	cl_sfx_ric1 = S_PrecacheSound("weapons/ric1.wav");
	cl_sfx_ric2 = S_PrecacheSound("weapons/ric2.wav");
	cl_sfx_ric3 = S_PrecacheSound("weapons/ric3.wav");
	cl_sfx_r_exp3 = S_PrecacheSound("weapons/r_exp3.wav");
}

void CL_ParseBeam(model_t *m)
{
	int ent = MSG_ReadShort();
	vec3_t start, end;
	start[0] = MSG_ReadCoord();
	start[1] = MSG_ReadCoord();
	start[2] = MSG_ReadCoord();
	end[0] = MSG_ReadCoord();
	end[1] = MSG_ReadCoord();
	end[2] = MSG_ReadCoord();
	// override any beam with the same entity
	beam_t *b = cl_beams;
	for (int i = 0; i < MAX_BEAMS; i++, b++)
		if (b->entity == ent) {
			b->entity = ent;
			b->model = m;
			b->endtime = cl.time + 0.2;
			VectorCopy(start, b->start);
			VectorCopy(end, b->end);
			return;
		}
	// find a free beam
	for (int i = 0; i < MAX_BEAMS; i++, b++)
		if (!b->model || b->endtime < cl.time) {
			b->entity = ent;
			b->model = m;
			b->endtime = cl.time + 0.2;
			VectorCopy(start, b->start);
			VectorCopy(end, b->end);
			return;
		}
	Con_Printf("beam list overflow!\n");
}

void CL_ParseTEnt()
{
	int type = MSG_ReadByte();
	vec3_t pos;
	dlight_t *dl;
	switch (type) {
	case TE_WIZSPIKE: // spike hitting wall
		pos[0] = MSG_ReadCoord();
		pos[1] = MSG_ReadCoord();
		pos[2] = MSG_ReadCoord();
		R_RunParticleEffect(pos, vec3_origin, 20, 30);
		S_StartSound(-1, 0, cl_sfx_wizhit, pos, 1, 1);
		break;
	case TE_KNIGHTSPIKE: // spike hitting wall
		pos[0] = MSG_ReadCoord();
		pos[1] = MSG_ReadCoord();
		pos[2] = MSG_ReadCoord();
		R_RunParticleEffect(pos, vec3_origin, 226, 20);
		S_StartSound(-1, 0, cl_sfx_knighthit, pos, 1, 1);
		break;
	case TE_SPIKE: // spike hitting wall
		pos[0] = MSG_ReadCoord();
		pos[1] = MSG_ReadCoord();
		pos[2] = MSG_ReadCoord();
		R_RunParticleEffect(pos, vec3_origin, 0, 10);
		if (rand() % 5)
			S_StartSound(-1, 0, cl_sfx_tink1, pos, 1, 1);
		else {
			int rnd = rand() & 3;
			if (rnd == 1)
				S_StartSound(-1, 0, cl_sfx_ric1, pos, 1, 1);
			else if (rnd == 2)
				S_StartSound(-1, 0, cl_sfx_ric2, pos, 1, 1);
			else
				S_StartSound(-1, 0, cl_sfx_ric3, pos, 1, 1);
		}
		break;
	case TE_SUPERSPIKE: // super spike hitting wall
		pos[0] = MSG_ReadCoord();
		pos[1] = MSG_ReadCoord();
		pos[2] = MSG_ReadCoord();
		R_RunParticleEffect(pos, vec3_origin, 0, 20);
		if (rand() % 5)
			S_StartSound(-1, 0, cl_sfx_tink1, pos, 1, 1);
		else {
			int rnd = rand() & 3;
			if (rnd == 1)
				S_StartSound(-1, 0, cl_sfx_ric1, pos, 1, 1);
			else if (rnd == 2)
				S_StartSound(-1, 0, cl_sfx_ric2, pos, 1, 1);
			else
				S_StartSound(-1, 0, cl_sfx_ric3, pos, 1, 1);
		}
		break;
	case TE_GUNSHOT: // bullet hitting wall
		pos[0] = MSG_ReadCoord();
		pos[1] = MSG_ReadCoord();
		pos[2] = MSG_ReadCoord();
		R_RunParticleEffect(pos, vec3_origin, 0, 20);
		break;
	case TE_EXPLOSION: // rocket explosion
		pos[0] = MSG_ReadCoord();
		pos[1] = MSG_ReadCoord();
		pos[2] = MSG_ReadCoord();
		R_ParticleExplosion(pos);
		dl = CL_AllocDlight(0);
		VectorCopy(pos, dl->origin);
		dl->radius = 350;
		dl->die = cl.time + 0.5;
		dl->decay = 300;
		S_StartSound(-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		break;
	case TE_TAREXPLOSION: // tarbaby explosion
		pos[0] = MSG_ReadCoord();
		pos[1] = MSG_ReadCoord();
		pos[2] = MSG_ReadCoord();
		R_BlobExplosion(pos);
		S_StartSound(-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		break;
	case TE_LIGHTNING1: // lightning bolts
		CL_ParseBeam(Mod_ForName("progs/bolt.mdl", true));
		break;
	case TE_LIGHTNING2: // lightning bolts
		CL_ParseBeam(Mod_ForName("progs/bolt2.mdl", true));
		break;
	case TE_LIGHTNING3: // lightning bolts
		CL_ParseBeam(Mod_ForName("progs/bolt3.mdl", true));
		break;
	case TE_BEAM: // grappling hook beam // PGM 01/21/97 
		CL_ParseBeam(Mod_ForName("progs/beam.mdl", true));
		break;
	case TE_LAVASPLASH: // PGM 01/21/97
		pos[0] = MSG_ReadCoord();
		pos[1] = MSG_ReadCoord();
		pos[2] = MSG_ReadCoord();
		R_LavaSplash(pos);
		break;
	case TE_TELEPORT:
		pos[0] = MSG_ReadCoord();
		pos[1] = MSG_ReadCoord();
		pos[2] = MSG_ReadCoord();
		R_TeleportSplash(pos);
		break;
	case TE_EXPLOSION2: // color mapped explosion
		pos[0] = MSG_ReadCoord();
		pos[1] = MSG_ReadCoord();
		pos[2] = MSG_ReadCoord();
		int colorStart = MSG_ReadByte();
		int colorLength = MSG_ReadByte();
		R_ParticleExplosion2(pos, colorStart, colorLength);
		dl = CL_AllocDlight(0);
		VectorCopy(pos, dl->origin);
		dl->radius = 350;
		dl->die = cl.time + 0.5;
		dl->decay = 300;
		S_StartSound(-1, 0, cl_sfx_r_exp3, pos, 1, 1);
		break;
	default:
		Sys_Error("CL_ParseTEnt: bad type");
	}
}

entity_t *CL_NewTempEntity(void)
{
	if (cl_numvisedicts == MAX_VISEDICTS
		|| num_temp_entities == MAX_TEMP_ENTITIES)
		return NULL;
	entity_t *ent = &cl_temp_entities[num_temp_entities];
	memset(ent, 0, sizeof(*ent));
	num_temp_entities++;
	cl_visedicts[cl_numvisedicts] = ent;
	cl_numvisedicts++;
	ent->colormap = vid.colormap;
	return ent;
}

void CL_UpdateTEnts()
{
	num_temp_entities = 0;
	beam_t *b = cl_beams;
	for (int i = 0; i < MAX_BEAMS; i++, b++) { // update lightning
		if (!b->model || b->endtime < cl.time)
			continue;
		// if coming from the player, update the start position
		if (b->entity == cl.viewentity) {
			VectorCopy(cl_entities[cl.viewentity].origin, b->start);
		}
		vec3_t dist; // calculate pitch and yaw
		VectorSubtract(b->end, b->start, dist);
		float yaw, pitch;
		if (dist[1] == 0 && dist[0] == 0) {
			yaw = 0;
			pitch = dist[2] > 0 ? 90 : 270;
		} else {
			yaw = (int)(atan2(dist[1], dist[0]) * 180 / M_PI);
			if (yaw < 0)
				yaw += 360;
			float forward = sqrt(dist[0] * dist[0] + dist[1] * dist[1]);
			pitch = (int)(atan2(dist[2], forward) * 180 / M_PI);
			if (pitch < 0)
				pitch += 360;
		}
		vec3_t org; // add new entities for the lightning
		VectorCopy(b->start, org);
		float d = VectorNormalize(dist);
		while (d > 0) {
			entity_t *ent = CL_NewTempEntity();
			if (!ent)
				return;
			VectorCopy(org, ent->origin);
			ent->model = b->model;
			ent->angles[0] = pitch;
			ent->angles[1] = yaw;
			ent->angles[2] = rand() % 360;
			for (int j = 0; j < 3; j++)
				org[j] += dist[j] * 30;
			d -= 30;
		}
	}
}
