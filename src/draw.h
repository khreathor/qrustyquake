// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

// draw.h -- these are the only functions outside the refresh allowed
// to touch the vid buffer

extern qpic_t *draw_disc; // also used on sbar

void Draw_Init ();
void Draw_CharacterScaled (int x, int y, int num, int scale);
void Draw_PicScaled (int x, int y, qpic_t *pic, int scale);
void Draw_TransPicScaled (int x, int y, qpic_t *pic, int scale);
void Draw_TransPicTranslateScaled (int x, int y, qpic_t *pic, byte *translation, int scale);
void Draw_ConsoleBackground (int lines);
void Draw_TileClear (int x, int y, int w, int h);
void Draw_Fill (int x, int y, int w, int h, int c);
void Draw_FadeScreen ();
void Draw_StringScaled (int x, int y, char *str, int scale);
qpic_t *Draw_PicFromWad (char *name);
qpic_t *Draw_CachePic (char *path);
