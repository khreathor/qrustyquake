// Copyright (C) 1996-2001 Id Software, Inc.
// Copyright (C) 2002-2009 John Fitzgibbons and others
// Copyright (C) 2010-2014 QuakeSpasm developers
// GPLv3 See LICENSE for details.
#ifndef MODEL_H
#define MODEL_H
extern aliashdr_t *pheader;
extern stvert_t stverts[MAXALIASVERTS];
extern mtriangle_t triangles[MAXALIASTRIS];
extern trivertx_t *poseverts[MAXALIASFRAMES];
void Mod_Init ();
void Mod_ClearAll ();
void Mod_ResetAll (); // for gamedir changes (Host_Game_f)
model_t *Mod_ForName (const s8 *name, bool crash);
void *Mod_Extradata (model_t *mod); // handles caching
void Mod_TouchModel (const s8 *name);
mleaf_t *Mod_PointInLeaf (vec3_t p, model_t *model);
byte *Mod_LeafPVS (mleaf_t *leaf, model_t *model);
byte *Mod_NoVisPVS (model_t *model);
void Mod_SetExtraFlags (model_t *mod);
#endif /* MODEL_H */
