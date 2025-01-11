// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

// cvar.c -- dynamic variable tracking

#include "quakedef.h"

cvar_t *cvar_vars;
char *cvar_null_string = "";

cvar_t *Cvar_FindVar(const char *var_name)
{
	for (cvar_t *var = cvar_vars; var; var = var->next)
		if (!Q_strcmp(var_name, var->name))
			return var;
	return NULL;
}

float   Cvar_VariableValue (char *var_name)
{
        cvar_t  *var;

        var = Cvar_FindVar (var_name);
        if (!var)
                return 0;
        return Q_atof (var->string);
}

char *Cvar_VariableString(char *var_name)
{
	cvar_t *var = Cvar_FindVar(var_name);
	if (!var)
		return cvar_null_string;
	return var->string;
}

char *Cvar_CompleteVariable(char *partial)
{
	int len = Q_strlen(partial);
	if (!len)
		return NULL;
	for (cvar_t *cvar = cvar_vars; cvar; cvar = cvar->next)
		if (!Q_strncmp(partial, cvar->name, len)) // check functions
			return cvar->name;
	return NULL;
}

void Cvar_Set(char *var_name, const char *value)
{
	cvar_t *var = Cvar_FindVar(var_name);
	if (!var) { // there is an error in C code if this happens
		Con_Printf("Cvar_Set: variable %s not found\n", var_name);
		return;
	}
	qboolean changed = Q_strcmp(var->string, value);
	Z_Free(var->string); // free the old value string
	var->string = Z_Malloc(Q_strlen(value) + 1);
	Q_strcpy(var->string, value);
	var->value = Q_atof(var->string);
	if (var->server && changed && sv.active)
		SV_BroadcastPrintf("\"%s\" changed to \"%s\"\n",
					   var->name, var->string);
}

void Cvar_SetValue(char *var_name, const float value)
{
	char val[32];
	sprintf(val, "%f", value);
	Cvar_Set(var_name, val);
}


void Cvar_RegisterVariable(cvar_t *variable)
{ // Adds a freestanding variable to the variable list.
	// first check to see if it has allready been defined
	if (Cvar_FindVar(variable->name)) {
		Con_Printf("Can't register variable %s, allready defined\n",
			   variable->name);
		return;
	}
	// check for overlap with a command
	if (Cmd_Exists(variable->name)) {
		Con_Printf("Cvar_RegisterVariable: %s is a command\n",
			   variable->name);
		return;
	}
	// copy the value off, because future sets will Z_Free it
	char *oldstr = variable->string;
	variable->string = Z_Malloc(Q_strlen(variable->string) + 1);
	Q_strcpy(variable->string, oldstr);
	variable->value = Q_atof(variable->string);
	variable->next = cvar_vars; // link the variable in
	cvar_vars = variable;
}

qboolean Cvar_Command()
{ // Handles variable inspection and changing from the console
	cvar_t *v = Cvar_FindVar(Cmd_Argv(0)); // check variables
	if (!v)
		return false;
	if (Cmd_Argc() == 1) { // perform a variable print or set
		Con_Printf("\"%s\" is \"%s\"\n", v->name, v->string);
		return true;
	}
	Cvar_Set(v->name, Cmd_Argv(1));
	return true;
}

void Cvar_WriteVariables(FILE *f) // Writes lines containing "set variable
{ // value" for all variables with the archive flag set to true.
	for (cvar_t *var = cvar_vars; var; var = var->next)
		if (var->archive)
			fprintf(f, "%s \"%s\"\n", var->name, var->string);
}
