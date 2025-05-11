// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

typedef enum { key_game, key_console, key_message, key_menu } keydest_t;

extern keydest_t key_dest;
extern char *keybindings[256];
extern int key_repeats[256];
extern int key_count; // incremented every key event
extern int key_lastpress;

void Key_Event(int key, qboolean down);
void Key_Init();
void Key_WriteBindings(FILE *f);
void Key_SetBinding(int keynum, char *binding);
