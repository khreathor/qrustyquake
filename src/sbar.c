// Copyright (C) 1996-1997 Id Software, Inc. GPLv3 See LICENSE for details.

#include "quakedef.h"

#define STAT_MINUS 10 // num frame for '-' stats digit

qpic_t *sb_nums[2][11];
qpic_t *sb_colon, *sb_slash;
qpic_t *sb_ibar;
qpic_t *sb_sbar;
qpic_t *sb_scorebar;
qpic_t *sb_weapons[7][8]; // 0 is active, 1 is owned, 2-5 are flashes
qpic_t *sb_ammo[4];
qpic_t *sb_sigil[4];
qpic_t *sb_armor[3];
qpic_t *sb_items[32];
qpic_t *sb_faces[7][2]; // 0 is gibbed, 1 is dead, 2-6 are alive, 0 is static, 1 is temporary animation
qpic_t *sb_face_invis;
qpic_t *sb_face_quad;
qpic_t *sb_face_invuln;
qpic_t *sb_face_invis_invuln;
qpic_t *rsb_invbar[2];
qpic_t *rsb_weapons[5];
qpic_t *rsb_items[2];
qpic_t *rsb_ammo[3];
qpic_t *rsb_teambord; // PGM 01/19/97 - team color border
      //MED 01/04/97 added two more weapons + 3 alternates for grenade launcher
qpic_t *hsb_weapons[7][5]; // 0 is active, 1 is owned, 2-5 are flashes
qboolean sb_showscores;
int sb_lines; // scan lines to draw
int hipweapons[4] = //MED 01/04/97 added array to simplify weapon parsing
{ HIT_LASER_CANNON_BIT, HIT_MJOLNIR_BIT, 4, HIT_PROXIMITY_GUN_BIT };
qpic_t *hsb_items[2]; //MED 01/04/97 added hipnotic items array
unsigned int sb_updates; // if >= vid.numpages, no update needed
int fragsort[MAX_SCOREBOARD];
char scoreboardtext[MAX_SCOREBOARD][20];
int scoreboardtop[MAX_SCOREBOARD];
int scoreboardbottom[MAX_SCOREBOARD];
int scoreboardcount[MAX_SCOREBOARD];
int scoreboardlines;
int npos[4][2]; // ammo count num
int wpos[9][2]; // weapons
int kpos[2][2]; // keys
int iposx[12]; // classic items
int iposy;

void Sbar_MiniDeathmatchOverlay();
void Sbar_DeathmatchOverlay();
void M_DrawPic(int x, int y, qpic_t * pic);

void Sbar_ShowScores() // Tab key down
{
	if (sb_showscores)
		return;
	sb_showscores = true;
	sb_updates = 0;
}


void Sbar_DontShowScores() // Tab key up
{
	sb_showscores = false;
	sb_updates = 0;
}

void Sbar_Changed()
{
	sb_updates = 0; // update next frame
}

void Sbar_Init()
{
	for (int i = 0; i < 10; i++) {
		sb_nums[0][i] = Draw_PicFromWad(va("num_%i", i));
		sb_nums[1][i] = Draw_PicFromWad(va("anum_%i", i));
	}
	sb_nums[0][10] = Draw_PicFromWad("num_minus");
	sb_nums[1][10] = Draw_PicFromWad("anum_minus");
	sb_colon = Draw_PicFromWad("num_colon");
	sb_slash = Draw_PicFromWad("num_slash");
	sb_weapons[0][0] = Draw_PicFromWad("inv_shotgun");
	sb_weapons[0][1] = Draw_PicFromWad("inv_sshotgun");
	sb_weapons[0][2] = Draw_PicFromWad("inv_nailgun");
	sb_weapons[0][3] = Draw_PicFromWad("inv_snailgun");
	sb_weapons[0][4] = Draw_PicFromWad("inv_rlaunch");
	sb_weapons[0][5] = Draw_PicFromWad("inv_srlaunch");
	sb_weapons[0][6] = Draw_PicFromWad("inv_lightng");
	sb_weapons[1][0] = Draw_PicFromWad("inv2_shotgun");
	sb_weapons[1][1] = Draw_PicFromWad("inv2_sshotgun");
	sb_weapons[1][2] = Draw_PicFromWad("inv2_nailgun");
	sb_weapons[1][3] = Draw_PicFromWad("inv2_snailgun");
	sb_weapons[1][4] = Draw_PicFromWad("inv2_rlaunch");
	sb_weapons[1][5] = Draw_PicFromWad("inv2_srlaunch");
	sb_weapons[1][6] = Draw_PicFromWad("inv2_lightng");
	for (int i = 0; i < 5; i++) {
		sb_weapons[2+i][0] = Draw_PicFromWad(va("inva%i_shotgun",i+1));
		sb_weapons[2+i][1] = Draw_PicFromWad(va("inva%i_sshotgun",i+1));
		sb_weapons[2+i][2] = Draw_PicFromWad(va("inva%i_nailgun",i+1));
		sb_weapons[2+i][3] = Draw_PicFromWad(va("inva%i_snailgun",i+1));
		sb_weapons[2+i][4] = Draw_PicFromWad(va("inva%i_rlaunch",i+1));
		sb_weapons[2+i][5] = Draw_PicFromWad(va("inva%i_srlaunch",i+1));
		sb_weapons[2+i][6] = Draw_PicFromWad(va("inva%i_lightng",i+1));
	}
	sb_ammo[0] = Draw_PicFromWad("sb_shells");
	sb_ammo[1] = Draw_PicFromWad("sb_nails");
	sb_ammo[2] = Draw_PicFromWad("sb_rocket");
	sb_ammo[3] = Draw_PicFromWad("sb_cells");
	sb_armor[0] = Draw_PicFromWad("sb_armor1");
	sb_armor[1] = Draw_PicFromWad("sb_armor2");
	sb_armor[2] = Draw_PicFromWad("sb_armor3");
	sb_items[0] = Draw_PicFromWad("sb_key1");
	sb_items[1] = Draw_PicFromWad("sb_key2");
	sb_items[2] = Draw_PicFromWad("sb_invis");
	sb_items[3] = Draw_PicFromWad("sb_invuln");
	sb_items[4] = Draw_PicFromWad("sb_suit");
	sb_items[5] = Draw_PicFromWad("sb_quad");
	sb_sigil[0] = Draw_PicFromWad("sb_sigil1");
	sb_sigil[1] = Draw_PicFromWad("sb_sigil2");
	sb_sigil[2] = Draw_PicFromWad("sb_sigil3");
	sb_sigil[3] = Draw_PicFromWad("sb_sigil4");
	sb_faces[4][0] = Draw_PicFromWad("face1");
	sb_faces[4][1] = Draw_PicFromWad("face_p1");
	sb_faces[3][0] = Draw_PicFromWad("face2");
	sb_faces[3][1] = Draw_PicFromWad("face_p2");
	sb_faces[2][0] = Draw_PicFromWad("face3");
	sb_faces[2][1] = Draw_PicFromWad("face_p3");
	sb_faces[1][0] = Draw_PicFromWad("face4");
	sb_faces[1][1] = Draw_PicFromWad("face_p4");
	sb_faces[0][0] = Draw_PicFromWad("face5");
	sb_faces[0][1] = Draw_PicFromWad("face_p5");
	sb_face_invis = Draw_PicFromWad("face_invis");
	sb_face_invuln = Draw_PicFromWad("face_invul2");
	sb_face_invis_invuln = Draw_PicFromWad("face_inv2");
	sb_face_quad = Draw_PicFromWad("face_quad");
	Cmd_AddCommand("+showscores", Sbar_ShowScores);
	Cmd_AddCommand("-showscores", Sbar_DontShowScores);
	sb_sbar = Draw_PicFromWad("sbar");
	sb_ibar = Draw_PicFromWad("ibar");
	sb_scorebar = Draw_PicFromWad("scorebar");
	if (hipnotic) { //MED 01/04/97 added new hipnotic weapons
		hsb_weapons[0][0] = Draw_PicFromWad("inv_laser");
		hsb_weapons[0][1] = Draw_PicFromWad("inv_mjolnir");
		hsb_weapons[0][2] = Draw_PicFromWad("inv_gren_prox");
		hsb_weapons[0][3] = Draw_PicFromWad("inv_prox_gren");
		hsb_weapons[0][4] = Draw_PicFromWad("inv_prox");
		hsb_weapons[1][0] = Draw_PicFromWad("inv2_laser");
		hsb_weapons[1][1] = Draw_PicFromWad("inv2_mjolnir");
		hsb_weapons[1][2] = Draw_PicFromWad("inv2_gren_prox");
		hsb_weapons[1][3] = Draw_PicFromWad("inv2_prox_gren");
		hsb_weapons[1][4] = Draw_PicFromWad("inv2_prox");
		hsb_items[0] = Draw_PicFromWad("sb_wsuit");
		hsb_items[1] = Draw_PicFromWad("sb_eshld");
	}
	for (int i = 0; hipnotic && i < 5; i++) {
		hsb_weapons[2+i][0]=Draw_PicFromWad(va("inva%i_laser",i+1));
		hsb_weapons[2+i][1]=Draw_PicFromWad(va("inva%i_mjolnir",i+1));
		hsb_weapons[2+i][2]=Draw_PicFromWad(va("inva%i_gren_prox",i+1));
		hsb_weapons[2+i][3]=Draw_PicFromWad(va("inva%i_prox_gren",i+1));
		hsb_weapons[2+i][4]=Draw_PicFromWad(va("inva%i_prox",i+1));
	}
	if (rogue) {
		rsb_invbar[0] = Draw_PicFromWad("r_invbar1");
		rsb_invbar[1] = Draw_PicFromWad("r_invbar2");
		rsb_weapons[0] = Draw_PicFromWad("r_lava");
		rsb_weapons[1] = Draw_PicFromWad("r_superlava");
		rsb_weapons[2] = Draw_PicFromWad("r_gren");
		rsb_weapons[3] = Draw_PicFromWad("r_multirock");
		rsb_weapons[4] = Draw_PicFromWad("r_plasma");
		rsb_items[0] = Draw_PicFromWad("r_shield1");
		rsb_items[1] = Draw_PicFromWad("r_agrav1");
		// PGM 01/19/97 - team color border
		rsb_teambord = Draw_PicFromWad("r_teambord");
		// PGM 01/19/97 - team color border
		rsb_ammo[0] = Draw_PicFromWad("r_ammolava");
		rsb_ammo[1] = Draw_PicFromWad("r_ammomulti");
		rsb_ammo[2] = Draw_PicFromWad("r_ammoplasma");
	}
}

// drawing routines are relative to the status bar location
void Sbar_DrawPic(int x, int y, qpic_t *pic)
{
	if (cl.gametype == GAME_DEATHMATCH)
		Draw_PicScaled(x * uiscale, y * uiscale + (vid.height
					- SBAR_HEIGHT * uiscale), pic, uiscale);
	else
		Draw_PicScaled(x * uiscale + ((vid.width - 320 * uiscale) >> 1),
			y * uiscale + (vid.height - SBAR_HEIGHT * uiscale),
			pic, uiscale);
}

void Sbar_DrawTransPic(int x, int y, qpic_t *pic)
{
	if (cl.gametype == GAME_DEATHMATCH)
		Draw_TransPicScaled(x * uiscale, y * uiscale + (vid.height
					- SBAR_HEIGHT * uiscale), pic, uiscale);
	else
		Draw_TransPicScaled(x*uiscale+((vid.width-320*uiscale)/2),
			y*uiscale+(vid.height-SBAR_HEIGHT*uiscale),pic,uiscale);
}

void Sbar_DrawCharacter(int x, int y, int num)
{ // Draws one solid graphics character
	if (cl.gametype == GAME_DEATHMATCH)
		Draw_CharacterScaled(x * uiscale + 4, y * uiscale + (vid.height
					- SBAR_HEIGHT * uiscale), num, uiscale);
	else
		Draw_CharacterScaled(x*uiscale+((vid.width-320*uiscale)/2)+4,
			y*uiscale+(vid.height-SBAR_HEIGHT*uiscale),num,uiscale);
}

void Sbar_DrawString(int x, int y, char *str)
{
	if (cl.gametype == GAME_DEATHMATCH)
		Draw_StringScaled(x * uiscale, y * uiscale + (vid.height -
					SBAR_HEIGHT * uiscale), str, uiscale);
	else
		Draw_StringScaled(x * uiscale + ((vid.width - 320 * uiscale) >>
					1), y * uiscale + (vid.height -
					SBAR_HEIGHT * uiscale), str, uiscale);
}

int Sbar_itoa(int num, char *buf)
{
	char *str = buf;
	if (num < 0) {
		*str++ = '-';
		num = -num;
	}
	int pow10 = 10;
	for (; num >= pow10; pow10 *= 10) ;
	do {
		pow10 /= 10;
		int dig = num / pow10;
		*str++ = '0' + dig;
		num -= dig * pow10;
	} while (pow10 != 1);
	*str = 0;
	return str - buf;
}

void Sbar_DrawNum(int x, int y, int num, int digits, int color)
{
	char str[12];
	char *ptr;
	int l, frame;
	l = Sbar_itoa(num, str);
	ptr = str;
	if (l > digits)
		ptr += (l - digits);
	if (l < digits)
		x += (digits - l) * 24 * uiscale;
	while (*ptr) {
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr - '0';
		Draw_TransPicScaled(x, y, sb_nums[color][frame], uiscale);
		x += 24 * uiscale;
		ptr++;
	}
}

void Sbar_DrawNumSmall(int x, int y, int num)
{
	char buf[6];
	sprintf(buf, "%3i", num);
	if (buf[0] != ' ')
		Draw_CharacterScaled(x, y, 18+buf[0]-'0', uiscale);
	if (buf[1] != ' ')
		Draw_CharacterScaled(x+8*uiscale, y, 18+buf[1]-'0', uiscale);
	if (buf[2] != ' ')
		Draw_CharacterScaled(x+16*uiscale, y, 18+buf[2]-'0', uiscale);
}

void Sbar_SortFrags()
{
	scoreboardlines = 0; // sort by frags
	for (int i = 0; i < cl.maxclients; i++) {
		if (cl.scores[i].name[0]) {
			fragsort[scoreboardlines] = i;
			scoreboardlines++;
		}
	}
	for (int i = 0; i < scoreboardlines; i++)
		for (int j = 0; j < scoreboardlines - 1 - i; j++)
			if (cl.scores[fragsort[j]].frags <
					cl.scores[fragsort[j + 1]].frags) {
				int k = fragsort[j];
				fragsort[j] = fragsort[j + 1];
				fragsort[j + 1] = k;
			}
}

int Sbar_ColorForMap(int m)
{
	return m < 128 ? m + 8 : m + 8;
}

void Sbar_SoloScoreboard()
{
	char str[80];
	sprintf(str, "Monsters:%3i /%3i", cl.stats[STAT_MONSTERS],
			cl.stats[STAT_TOTALMONSTERS]);
	Sbar_DrawString(8, 4, str);
	sprintf(str, "Secrets :%3i /%3i", cl.stats[STAT_SECRETS],
			cl.stats[STAT_TOTALSECRETS]);
	Sbar_DrawString(8, 12, str);
	int minutes = cl.time / 60; // time
	int seconds = cl.time - 60 * minutes;
	int tens = seconds / 10;
	int units = seconds - 10 * tens;
	sprintf(str, "Time :%3i:%i%i", minutes, tens, units);
	Sbar_DrawString(184, 4, str);
	int l = strlen(cl.levelname); // draw level name
	Sbar_DrawString(232 - l * 4, 12, cl.levelname);
}

void Sbar_DrawScoreboard()
{
	Sbar_SoloScoreboard();
	if (cl.gametype == GAME_DEATHMATCH)
		Sbar_DeathmatchOverlay();
}

void Sbar_CalcPos()
{
	for (int i = 0; i < 8; i++) // items classic
		iposx[i] = vid.width / 2 + 32*uiscale + i*16*uiscale;
	for (int i = 8; i < 12; i++)
		iposx[i] = vid.width / 2 + 64*uiscale + i*8*uiscale;
	iposy = vid.height - 40*uiscale;
	switch ((int)(hipnotic&&scr_hudstyle.value==3?2:scr_hudstyle.value)) {
	default: // ammo counters
	case 0: // classic
		for (int i = 0; i < 4; i++) {
			npos[i][0] = vid.width / 2 - 152*uiscale + i*48*uiscale;
			npos[i][1] = vid.height - 48*uiscale;
		}
		break;
	case 1: // bottom center, 4x1
		for (int i = 0; i < 4; i++) {
			npos[i][0] = vid.width / 2 - 88*uiscale + i*48*uiscale;
			npos[i][1] = vid.height - 8 * uiscale;
		}
		break;
	case 2: // right side, 2x2
		for (int i = 0; i < 4; i++) {
			npos[i][0] = vid.width - (scr_hudstyle.value==3?88:96)
				*uiscale + 48*uiscale*(i&1);
			npos[i][1] = vid.height - (scr_hudstyle.value==3?39:46)
				*uiscale - 9*uiscale*(i>>1);
		}
		break;
	case 3: // right side, 1x4
		for (int i = 0; i < 4; i++) {
			npos[i][0] = vid.width - 48*uiscale + 8*uiscale;
			npos[i][1] = vid.height - 78*uiscale + i * 9*uiscale;
		}
		break;
	}
	for (int i = 0; !scr_hudstyle.value && i < 9; i++) { // weapons classic
		wpos[i][0] = vid.width / 2 - 160*uiscale + i*24*uiscale;	
		wpos[i][1] = vid.height - 40*uiscale;
		if (i == 9) {
			wpos[7][0] += 8*uiscale;
			wpos[8][0] += 8*uiscale;
		}
	}
	for (int i = 0; scr_hudstyle.value && i < 9; i++) { // weapons modern
		wpos[i][0] = vid.width - 18*uiscale;	
		wpos[i][1] = vid.height- 16*uiscale*(6+i) + hipnotic*24*uiscale;
	}
	switch ((int)scr_hudstyle.value + 0x10*hipnotic) { // keys
	default:
	case 0:
	case 3:
		for (int i = 0; i < 2; i++) { // classic, qw
			kpos[i][0] = vid.width / 2 + 32*uiscale + i*16*uiscale;
			kpos[i][1] = vid.height - 40*uiscale;
		}
		break;
	case 1:
		for (int i = 0; i < 2; i++) { // side, vertical
			kpos[i][0] = vid.width - 28*uiscale;
			kpos[i][1] = vid.height - 52*uiscale - i*16*uiscale;
		}
		break;
	case 2:
		for (int i = 0; i < 2; i++) { // side, horizontal
			kpos[i][0] = vid.width - 24*uiscale - i*16*uiscale;
			kpos[i][1] = vid.height - 75*uiscale;
		}
		break;
	case 0x10: // hipnotic
	case 0x13:
		for (int i = 0; i < 2; i++) { // classic, qw
			kpos[i][0] = vid.width / 2 + 48*uiscale;
			kpos[i][1] = vid.height - 21*uiscale + i*8*uiscale;
		}
		break;
	case 0x11:
		for (int i = 0; i < 2; i++) { // side, vertical
			kpos[i][0] = vid.width - 28*uiscale;
			kpos[i][1] = vid.height - 44*uiscale - i*8*uiscale;
		}
		break;
	case 0x12:
		for (int i = 0; i < 2; i++) { // side, horizontal
			kpos[i][0] = vid.width - 40*uiscale - i*16*uiscale;
			kpos[i][1] = vid.height - 68*uiscale;
		}
		break;
	}
}

void Sbar_ItemsClassic()
{
	for (int i = 0; i < 6; i++)
		if (cl.items & (1 << (17 + i))) 
			Draw_PicScaled(iposx[i],iposy,sb_items[i],uiscale);
	for (int i = 0; hipnotic && i < 2; i++)
		if (cl.items & (1 << (24 + i)))
			Draw_PicScaled(iposx[i+6],iposy,hsb_items[i],uiscale);
	for (int i = 0; rogue && i < 2; i++)
		if (cl.items & (1 << (29 + i)))
			Draw_PicScaled(iposx[i+6],iposy,rsb_items[i],uiscale);
	for (int i = 0; !rogue && i < 4; i++) // sigils
		if (cl.items & (1 << (28 + i)))
			Draw_PicScaled(iposx[8+i], iposy, sb_sigil[i], uiscale);
}

void Sbar_ItemsModern()
{
	int x = 12 * uiscale;
	int y = vid.height - 48 * uiscale;
	if (cl.items & IT_INVULNERABILITY || cl.stats[STAT_ARMOR] > 0)
		y -= 24*uiscale; // armor is visible, start above it
	for (int i = 2; i < 6; i++)
		if ((cl.items & (1<<(17+i))) && (!hipnotic || (i > 1))) {
			Draw_PicScaled(x, y, sb_items[i], uiscale);
			y -= 16*uiscale;
		}
	for (int i = 0; hipnotic && i < 2; i++)
		if (cl.items & (1<<(24+i))) {
			Draw_PicScaled(x, y, hsb_items[i], uiscale);
			y -= 16*uiscale;
		}
	for (int i = 0; rogue && i < 2; i++)
		if (cl.items & (1<<(29+i))) {
			Draw_PicScaled(x, y, rsb_items[i], uiscale);
			y -= 16*uiscale;
		}
}

void Sbar_DrawInventoryBg()
{
	qpic_t *pic = rogue ?
		rsb_invbar[cl.stats[STAT_ACTIVEWEAPON] < RIT_LAVA_NAILGUN] :
		sb_ibar;
	int x, y;
	switch ((int)(hipnotic&&scr_hudstyle.value==3?2:scr_hudstyle.value)) {
	default:
	case 0: // classic
		x = vid.width / 2 - 160*uiscale;
		y = vid.height - 48*uiscale;
		Draw_PicScaled(x, y, pic, uiscale);
		break;
	case 1: // bottom center, 4x1
		x = vid.width / 2 - 96*uiscale;
		y = vid.height - 8*uiscale;
		Draw_PicScaledPartial(x, y, 0, 0, 192, 8, pic, uiscale);
		break;
	case 2: // right side, 2x2
		x = vid.width - (scr_hudstyle.value==3 ? 96 : 104)*uiscale;
		y = vid.height - (scr_hudstyle.value==3 ? 48 : 55)*uiscale;
		Draw_PicScaledPartial(x, y + 9*uiscale, 0,0,96,8, pic, uiscale);
		Draw_PicScaledPartial(x - 96*uiscale,y,96,8,192,16,pic,uiscale);
		break;
	case 3: // right side, 1x4
		x = vid.width - 48*uiscale;
		y = vid.height - 78*uiscale;
		for (int i = 0; i < 4; i++)
			Draw_PicScaledPartial(x - 48*uiscale*i, y + 9*uiscale*i,
					i*48, 0, (i+1)*48, 8, pic, uiscale);
		break;
	}
}

void Sbar_DrawWeapons()
{
	for (int i = 0; i < 7; i++) {
		if (hipnotic && i == IT_GRENADE_LAUNCHER)
			continue;
		if (cl.items & (1<<i)) {
			int active = (cl.stats[STAT_ACTIVEWEAPON] == (1<<i));
			float time = cl.item_gettime[i];
			int flashon = (int)((cl.time - time)*10);
			if (flashon >= 10)
				flashon = active;
			else
				flashon = (flashon%5) + 2;
			qpic_t *pic = sb_weapons[flashon][i];
			if (rogue && i >= 2 && cl.stats[STAT_ACTIVEWEAPON] == (RIT_LAVA_NAILGUN << (i - 2))) {
				pic = rsb_weapons[i - 2]; // powered up weapon
				active = 1;
			}
			active = scr_hudstyle.value ? active * 6 * uiscale : 0;
			Draw_PicScaled(wpos[i][0] - active, wpos[i][1], pic, uiscale);
			if (flashon > 1)
				sb_updates = 0; // force update to remove flash
		}
	}
	int grenadeflashing = 0;
	for (int i = 0; hipnotic && i < 4; i++) {
		if (!(cl.items & (1 << hipweapons[i])))
			continue;
		float time = cl.item_gettime[hipweapons[i]];
		int flashon = (int)((cl.time - time) * 10);
		int active = (cl.stats[STAT_ACTIVEWEAPON] == 1<<hipweapons[i]);
		active = scr_hudstyle.value ? active * 6 * uiscale : 0;
		flashon = flashon >= 10 ?
			(cl.stats[STAT_ACTIVEWEAPON] == (1 << hipweapons[i])) :
			(flashon % 5) + 2;
		if (i == 2) { // check grenade launcher
			if (cl.items & HIT_PROXIMITY_GUN && flashon) {
				grenadeflashing = 1;
				Draw_PicScaled(wpos[4][0]-active, wpos[4][1], hsb_weapons[flashon][2], uiscale);
			}
		} else if (i == 3) {
			if (cl.items & (1 << 4)) {
				if (flashon && !grenadeflashing)
					Draw_PicScaled(wpos[4][0]-active, wpos[4][1], hsb_weapons[flashon][3], uiscale);
				else if (!grenadeflashing)
					Draw_PicScaled(wpos[4][0]-active, wpos[4][1], hsb_weapons[0][3], uiscale);
			} else
				Draw_PicScaled(wpos[4][0]-active, wpos[4][1], hsb_weapons [flashon][4], uiscale);
		} else
			Draw_PicScaled(wpos[7+i][0]-active, wpos[7+i][1], hsb_weapons[flashon][i], uiscale);
		if (flashon > 1)
			sb_updates = 0; // force update to remove flash
	}
}

void Sbar_DrawInventory()
{
	Sbar_DrawInventoryBg();
	for (int i = 0; i < 4; i++) // ammo counters
		Sbar_DrawNumSmall(npos[i][0], npos[i][1], cl.stats[6 + i]);
	Sbar_DrawWeapons();
	for (int i = 0; i < 2; i++) { // keys
		if (!(cl.items & (1<<(17+i))))
			continue;
		Draw_PicScaled(kpos[i][0], kpos[i][1], sb_items[i], uiscale);
	}
	switch ((int)scr_hudstyle.value) {
	default: case 0: case 3: Sbar_ItemsClassic(); break;
	         case 1: case 2: Sbar_ItemsModern();  break;
	}
}

void Sbar_DrawFrags()
{
	char num[12];
	Sbar_SortFrags();
	int l = scoreboardlines <= 4 ? scoreboardlines : 4; // draw the text
	int x = 23; // nudge a bit to the right on higher scales
	int xofs = cl.gametype==GAME_DEATHMATCH? uiscale-1 : (vid.width-320)>>1;
	int y = vid.height - SBAR_HEIGHT * uiscale - 23 * uiscale;
	for (int i = 0; i < l; i++) {
		int k = fragsort[i];
		scoreboard_t *s = &cl.scores[k];
		if (!s->name[0])
			continue;
		int top = s->colors & 0xf0; // draw background
		int bottom = (s->colors & 15) << 4;
		top = Sbar_ColorForMap(top);
		bottom = Sbar_ColorForMap(bottom);
		Draw_Fill(xofs + x * 8 * uiscale + 10 * uiscale,
			y, 28 * uiscale, 4 * uiscale, top);
		Draw_Fill(xofs + x * 8 * uiscale + 10 * uiscale,
			y + 4 * uiscale, 28 * uiscale, 3 * uiscale, bottom);
		int f = s->frags; // draw number
		sprintf(num, "%3i", f);
		Sbar_DrawCharacter((x + 1) * 8, -24, num[0]);
		Sbar_DrawCharacter((x + 2) * 8, -24, num[1]);
		Sbar_DrawCharacter((x + 3) * 8, -24, num[2]);
		if (k == cl.viewentity - 1) {
			Sbar_DrawCharacter(x*8+2+(uiscale-1)*2,-24,16);
			Sbar_DrawCharacter((x+4)*8-4+(uiscale-1)*2,-24,17);
		}
		x += 4;
	}
}

void Sbar_DrawFace()
{
	int f, anim;
	int x = vid.width/2 - 48*uiscale; // classic, qw
	int y = vid.height - 24 * uiscale;
	if (scr_hudstyle.value == 1 || scr_hudstyle.value == 2) {
		x = 8 * uiscale;
		y = vid.height - 32 * uiscale;
	}
	// PGM 01/19/97 - team color drawing
	// PGM 03/02/97 - fixed so color swatch only appears in CTF modes
	if (rogue && (cl.maxclients != 1) && (teamplay.value > 3) &&
			(teamplay.value < 7)) {
		char num[12];
		scoreboard_t *s = &cl.scores[cl.viewentity - 1];
		int top = s->colors & 0xf0; // draw background
		int bottom = (s->colors & 15) << 4;
		top = Sbar_ColorForMap(top);
		bottom = Sbar_ColorForMap(bottom);
		int xofs = cl.gametype == GAME_DEATHMATCH ? 113 :
			((vid.width - 320) >> 1) + 113;
		Sbar_DrawPic(x, y, rsb_teambord);
		Draw_Fill(xofs, vid.height - SBAR_HEIGHT + 3, 22, 9, top);
		Draw_Fill(xofs, vid.height - SBAR_HEIGHT + 12, 22, 9, bottom);
		f = s->frags; // draw number
		sprintf(num, "%3i", f);
		if (top == 8) {
			if (num[0] != ' ')
				Sbar_DrawCharacter(109, 3, 18 + num[0] - '0');
			if (num[1] != ' ')
				Sbar_DrawCharacter(116, 3, 18 + num[1] - '0');
			if (num[2] != ' ')
				Sbar_DrawCharacter(123, 3, 18 + num[2] - '0');
		} else {
			Sbar_DrawCharacter(109, 3, num[0]);
			Sbar_DrawCharacter(116, 3, num[1]);
			Sbar_DrawCharacter(123, 3, num[2]);
		}
		return;
	}
	// PGM 01/19/97 - team color drawing
	if ((cl.items & (IT_INVISIBILITY | IT_INVULNERABILITY))
			== (IT_INVISIBILITY | IT_INVULNERABILITY)) {
		Draw_PicScaled(x, y, sb_face_invis_invuln, uiscale);
		return;
	}
	if (cl.items & IT_QUAD) {
		Draw_PicScaled(x, y, sb_face_quad, uiscale);
		return;
	}
	if (cl.items & IT_INVISIBILITY) {
		Draw_PicScaled(x, y, sb_face_invis, uiscale);
		return;
	}
	if (cl.items & IT_INVULNERABILITY) {
		Draw_PicScaled(x, y, sb_face_invuln, uiscale);
		return;
	}
	if (cl.stats[STAT_HEALTH] >= 100)
		f = 4;
	else
		f = cl.stats[STAT_HEALTH] / 20;
	if (cl.time <= cl.faceanimtime) {
		anim = 1;
		sb_updates = 0; // make sure the anim gets drawn over
	} else
		anim = 0;
	Draw_PicScaled(x, y, sb_faces[f][anim], uiscale);
}

void Sbar_Draw()
{
	if (scr_con_current == vid.height)
		return; // console is full screen
	if (sb_updates >= vid.numpages && !scr_hudstyle.value)
		return;
	scr_copyeverything = 1;
	sb_updates++;
	if (sb_lines && vid.width > 320)
		Draw_TileClear(0, vid.height - sb_lines, vid.width, sb_lines);
	if (scr_hudstyle.value || sb_lines > 24) {
		if (cl.maxclients != 1)
			Sbar_DrawFrags();
	}
	if (sb_showscores || cl.stats[STAT_HEALTH] <= 0) {
		Sbar_DrawPic(0, 0, sb_scorebar);
		Sbar_DrawScoreboard();
		sb_updates = 0;
	} else if (scr_hudstyle.value || sb_lines) {
		if (!scr_hudstyle.value)
			Draw_PicScaled(vid.width/2-160*(uiscale), vid.height-SBAR_HEIGHT*uiscale, sb_sbar, uiscale);
		int x = vid.width/2-160*(uiscale); // classic, qw
		int y = vid.height - 24*uiscale;
		if (scr_hudstyle.value == 1 || scr_hudstyle.value == 2) {
			x = 8 * uiscale;
			y = vid.height - 56 * uiscale;
		}
		if (cl.items & IT_INVULNERABILITY) { // armor
			Sbar_DrawNum(24*uiscale + x, y, 666, 3, 1);
			Draw_PicScaled(x, y, draw_disc, uiscale);
		} else {
			if (rogue) {
				if (!scr_hudstyle.value || cl.stats[STAT_ARMOR])
					Sbar_DrawNum(24*uiscale + x, y, cl.stats[STAT_ARMOR], 3,
							cl.stats[STAT_ARMOR] <= 25);
				if (cl.items & RIT_ARMOR3)
					Draw_PicScaled(x, y, sb_armor[2], uiscale);
				else if (cl.items & RIT_ARMOR2)
					Draw_PicScaled(x, y, sb_armor[1], uiscale);
				else if (cl.items & RIT_ARMOR1)
					Draw_PicScaled(x, y, sb_armor[0], uiscale);
			} else {
				if (!scr_hudstyle.value || cl.stats[STAT_ARMOR])
					Sbar_DrawNum(24*uiscale + x, y, cl.stats[STAT_ARMOR], 3,
							cl.stats[STAT_ARMOR] <= 25);
				if (cl.items & IT_ARMOR3)
					Draw_PicScaled(x, y, sb_armor[2], uiscale);
				else if (cl.items & IT_ARMOR2)
					Draw_PicScaled(x, y, sb_armor[1], uiscale);
				else if (cl.items & IT_ARMOR1)
					Draw_PicScaled(x, y, sb_armor[0], uiscale);
			}
		}
		x = vid.width/2-160*(uiscale) + 136*uiscale; // classic, qw
		y = vid.height - 24*uiscale;
		if (scr_hudstyle.value == 1 || scr_hudstyle.value == 2) {
			x = 32*uiscale;
			y = vid.height - 32*uiscale;
		}
		Sbar_DrawFace(); // face
		Sbar_DrawNum(x, y, cl.stats[STAT_HEALTH], 3, // health
				cl.stats[STAT_HEALTH] <= 25);
		x = vid.width/2-160*(uiscale) + 224*uiscale; // classic, qw
		y = vid.height - 24*uiscale;
		if (scr_hudstyle.value == 1 || scr_hudstyle.value == 2) {
			x = vid.width - 32*uiscale;
			y = vid.height - 32*uiscale;
		}
		if (rogue) { // ammo icon
			if (cl.items & RIT_SHELLS)
				Draw_TransPicScaled(x, y, sb_ammo[0], uiscale);
			else if (cl.items & RIT_NAILS)
				Draw_TransPicScaled(x, y, sb_ammo[1], uiscale);
			else if (cl.items & RIT_ROCKETS)
				Draw_TransPicScaled(x, y, sb_ammo[2], uiscale);
			else if (cl.items & RIT_CELLS)
				Draw_TransPicScaled(x, y, sb_ammo[3], uiscale);
			else if (cl.items & RIT_LAVA_NAILS)
				Draw_TransPicScaled(x, y, rsb_ammo[0], uiscale);
			else if (cl.items & RIT_PLASMA_AMMO)
				Draw_TransPicScaled(x, y, rsb_ammo[1], uiscale);
			else if (cl.items & RIT_MULTI_ROCKETS)
				Draw_TransPicScaled(x, y, rsb_ammo[2], uiscale);
		} else {
			if (cl.items & IT_SHELLS)
				Draw_TransPicScaled(x, y, sb_ammo[0], uiscale);
			else if (cl.items & IT_NAILS)
				Draw_TransPicScaled(x, y, sb_ammo[1], uiscale);
			else if (cl.items & IT_ROCKETS)
				Draw_TransPicScaled(x, y, sb_ammo[2], uiscale);
			else if (cl.items & IT_CELLS)
				Draw_TransPicScaled(x, y, sb_ammo[3], uiscale);
		}
		x = vid.width/2-160*(uiscale) + 248*uiscale; // classic, qw
		y = vid.height - 24*uiscale;
		if (scr_hudstyle.value == 1 || scr_hudstyle.value == 2) {
			x = vid.width - 108 * uiscale;
			y = vid.height - 32 * uiscale;
		}
		Sbar_DrawNum(x,y,cl.stats[STAT_AMMO],3,cl.stats[STAT_AMMO]<=10);
		Sbar_DrawInventory();
	}
	if (vid.width > 320 && cl.gametype == GAME_DEATHMATCH)
		Sbar_MiniDeathmatchOverlay();
}

void Sbar_IntermissionNumber(int x, int y, int num, int digits, int color)
{
	char str[12];
	int l = Sbar_itoa(num, str);
	char *ptr = str;
	if (l > digits)
		ptr += (l - digits);
	if (l < digits)
		x += (digits - l) * 24 * uiscale;
	while (*ptr) {
		int frame = *ptr == '-' ? STAT_MINUS : *ptr - '0';
		Draw_TransPicScaled(x, y, sb_nums[color][frame], uiscale);
		x += 24 * uiscale;
		ptr++;
	}
}

void Sbar_DeathmatchOverlay()
{
	char num[12];
	scr_copyeverything = 1;
	scr_fullupdate = 0;
	qpic_t *pic = Draw_CachePic("gfx/ranking.lmp");
	M_DrawPic((320 - pic->width) / 2, 8, pic);
	Sbar_SortFrags(); // scores
	int l = scoreboardlines; // draw the text
	int x = 80 * uiscale + ((vid.width - 320 * uiscale) >> 1);
	int y = 40 * uiscale;
	for (int i = 0; i < l; i++) {
		int k = fragsort[i];
		scoreboard_t *s = &cl.scores[k];
		if (!s->name[0])
			continue;
		int top = s->colors & 0xf0; // draw background
		int bottom = (s->colors & 15) << 4;
		top = Sbar_ColorForMap(top);
		bottom = Sbar_ColorForMap(bottom);
		Draw_Fill(x, y, 40 * uiscale, 4 * uiscale, top);
		Draw_Fill(x, y + 4 * uiscale, 40 * uiscale, 4 * uiscale,
				bottom);
		int f = s->frags; // draw number
		sprintf(num, "%3i", f);
		Draw_CharacterScaled(x + 8 * uiscale, y, num[0], uiscale);
		Draw_CharacterScaled(x + 16 * uiscale, y, num[1], uiscale);
		Draw_CharacterScaled(x + 24 * uiscale, y, num[2], uiscale);
		if (k == cl.viewentity - 1)
			Draw_CharacterScaled(x - 8 * uiscale, y, 12, uiscale);
		Draw_StringScaled(x + 64 * uiscale, y, s->name, uiscale); //name
		y += 10 * uiscale;
	}
}

void Sbar_MiniDeathmatchOverlay()
{
	char num[12];
	if (vid.width < 512 || !sb_lines || uiscale > 1)
		return;
	scr_copyeverything = 1;
	scr_fullupdate = 0;
	Sbar_SortFrags(); // scores
	unsigned int y = vid.height - sb_lines; // draw the text
	int numlines = sb_lines / 8;
	if (numlines < 3)
		return;
	int i = 0;
	for (; i < scoreboardlines; i++) // find us
		if (fragsort[i] == cl.viewentity - 1)
			break;
	if (i == scoreboardlines) // we're not there
		i = 0;
	else // figure out start
		i = i - numlines / 2;
	if (i > scoreboardlines - numlines)
		i = scoreboardlines - numlines;
	if (i < 0)
		i = 0;
	int x = 324 * uiscale;
	for (; i < scoreboardlines && y < vid.height - 8; i++) {
		int k = fragsort[i];
		scoreboard_t *s = &cl.scores[k];
		if (!s->name[0])
			continue;
		int top = s->colors & 0xf0; // draw background
		int bottom = (s->colors & 15) << 4;
		top = Sbar_ColorForMap(top);
		bottom = Sbar_ColorForMap(bottom);
		Draw_Fill(x, y + 1, 40, 3, top);
		Draw_Fill(x, y + 4, 40, 4, bottom);
		int f = s->frags; // draw number
		sprintf(num, "%3i", f);
		Draw_CharacterScaled(x + 8, y, num[0], uiscale);
		Draw_CharacterScaled(x + 16, y, num[1], uiscale);
		Draw_CharacterScaled(x + 24, y, num[2], uiscale);
		if (k == cl.viewentity - 1) {
			Draw_CharacterScaled(x, y, 16, uiscale);
			Draw_CharacterScaled(x + 32, y, 17, uiscale);
		} // draw name
		Draw_StringScaled(x + 48 * uiscale, y, s->name, uiscale);
		y += 8;
	}
}

void Sbar_IntermissionOverlay()
{
	scr_copyeverything = 1;
	scr_fullupdate = 0;
	if (cl.gametype == GAME_DEATHMATCH) {
		Sbar_DeathmatchOverlay();
		return;
	}
	qpic_t *pic = Draw_CachePic("gfx/complete.lmp"); // plaque is 192px wide
	Draw_PicScaled((vid.width - 192 * uiscale) / 2,24*uiscale,pic,uiscale);
	int p = (vid.width - 320 * uiscale) / 2; // padding for scaling
	pic = Draw_CachePic("gfx/inter.lmp");
	Draw_TransPicScaled(p, 56 * uiscale, pic, uiscale);
	int dig = cl.completed_time / 60; // time
	Sbar_IntermissionNumber(vid.width / 2, 64 * uiscale, dig, 3, 0);
	int num = cl.completed_time - dig * 60;
	Draw_TransPicScaled(vid.width-p-86*uiscale,64*uiscale,sb_colon,uiscale);
	Draw_TransPicScaled(vid.width-p-74*uiscale,64*uiscale,sb_nums[0][num/10],uiscale);
	Draw_TransPicScaled(vid.width-p-54*uiscale,64*uiscale,sb_nums[0][num%10],uiscale);
	Sbar_IntermissionNumber(vid.width/2,104*uiscale,cl.stats[STAT_SECRETS],3,0);
	Draw_TransPicScaled(vid.width-p-88*uiscale,104*uiscale,sb_slash,uiscale);
	Sbar_IntermissionNumber(vid.width-p-80*uiscale,104*uiscale,cl.stats[STAT_TOTALSECRETS],3,0);
	Sbar_IntermissionNumber(vid.width/2,144*uiscale,cl.stats[STAT_MONSTERS],3,0);
	Draw_TransPicScaled(vid.width-p-88*uiscale,144*uiscale,sb_slash,uiscale);
	Sbar_IntermissionNumber(vid.width-p-80*uiscale,144*uiscale,cl.stats[STAT_TOTALMONSTERS],3,0);
}

void Sbar_FinaleOverlay()
{
	scr_copyeverything = 1;
	qpic_t *pic = Draw_CachePic("gfx/finale.lmp");
	Draw_TransPicScaled((vid.width - (pic->width) * uiscale) / 2,
			16 * uiscale, pic, uiscale);
}
