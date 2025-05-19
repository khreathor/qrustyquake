#include "quakedef.h"

static f32 fog_red; // CyanBun96: we store the actual RGB values in these,
static f32 fog_green; // but they get quantized to a single index in the color
static f32 fog_blue; // palette before use, stored in fog_pal_index
static f32 randarr[RANDARR_SIZE];
static u32 lfsr = 0x1337; // non-zero seed

void Fog_SetPalIndex(cvar_t */*cvar*/)
{
	fog_pal_index = rgbtoi_lab(fog_red * 255.0f*r_fogbrightness.value,
				   fog_green*255.0f*r_fogbrightness.value,
				   fog_blue *255.0f*r_fogbrightness.value);
}

void Fog_Update(f32 d, f32 r, f32 g, f32 b) {
	if     (d < 0.0f) d = 0.0f;
	if     (r < 0.0f) r = 0.0f;
	else if(r > 1.0f) r = 1.0f;
	if     (g < 0.0f) g = 0.0f;
	else if(g > 1.0f) g = 1.0f;
	if     (b < 0.0f) b = 0.0f;
	else if(b > 1.0f) b = 1.0f;
	fog_density = d;
	fog_red = r;
	fog_green = g;
	fog_blue = b;
	Fog_SetPalIndex(0);
	vid.recalc_refdef = 1;
	fog_initialized = 0;
}

void Fog_FogCommand_f () // yanked from Quakespasm, mostly
{ // TODO time transitions
	f32 d, r, g, b;
	switch(Cmd_Argc()){
	default:
	case 1:
		Con_Printf("usage:\n");
		Con_Printf("   fog <density>\n");
		Con_Printf("   fog <red> <green> <blue>\n");
		Con_Printf("   fog <density> <red> <green> <blue>\n");
		Con_Printf("current values:\n");
		Con_Printf("   \"density\" is \"%f\"\n", fog_density);
		Con_Printf("   \"red\" is \"%f\"\n", fog_red);
		Con_Printf("   \"green\" is \"%f\"\n", fog_green);
		Con_Printf("   \"blue\" is \"%f\"\n", fog_blue);
		return;
	case 2:
		d = Q_atof(Cmd_Argv(1));
		r = fog_red;
		g = fog_green;
		b = fog_blue;
		break;
	case 3: //TEST
		d = Q_atof(Cmd_Argv(1));
		r = fog_red;
		g = fog_green;
		b = fog_blue;
		break;
	case 4:
		d = fog_density;
		r = Q_atof(Cmd_Argv(1));
		g = Q_atof(Cmd_Argv(2));
		b = Q_atof(Cmd_Argv(3));
		break;
	case 5:
		d = Q_atof(Cmd_Argv(1));
		r = Q_atof(Cmd_Argv(2));
		g = Q_atof(Cmd_Argv(3));
		b = Q_atof(Cmd_Argv(4));
		break;
	}
	Fog_Update(d, r, g, b);
}

void Fog_ParseWorldspawn () // from Quakespasm
{ // called at map load
	s8 key[128], value[4096];
	fog_density = fog_red = fog_green = fog_blue = 0;
	const s8 *data = COM_Parse(cl.worldmodel->entities);
	if(!data || com_token[0] != '{') return; // error
	while(1){
		if(!data) return; // error
		if(com_token[0] == '}') break; // end of worldspawn
		if(com_token[0] == '_')q_strlcpy(key, com_token+1, sizeof(key));
		else q_strlcpy(key, com_token, sizeof(key));
		while(key[0] && key[strlen(key)-1] == ' ') // no trailing spaces
			key[strlen(key)-1] = 0;
		data = COM_ParseEx(data, CPE_ALLOWTRUNC);
		if(!data) return; // error
		q_strlcpy(value, com_token, sizeof(value));
		if(!strcmp("fog", key))
			sscanf(value, "%f %f %f %f", &fog_density, &fog_red,
					&fog_green, &fog_blue);
	}
	Fog_SetPalIndex(0);
}

u32 lfsr_random() {lfsr^=lfsr>>7; lfsr^=lfsr<<9; lfsr^=lfsr>>13; return lfsr;}

void R_InitFog()
{
	for(s32 i = 0; i < RANDARR_SIZE; ++i) // fog bias array
		randarr[i] = (lfsr_random() & 0xFFFF) / 65535.0f; // == [0,1]
	if(!fog_lut_built) build_color_mix_lut(0);
	Fog_SetPalIndex(0);
	fog_initialized = 1;
}

f32 compute_fog(s32 z)
{
	s32 fog_scale = 18 * r_fogscale.value;
	z = fog_scale>z?fog_scale:z; // prevent distant objects getting no fog
	z /= fog_scale;
	return expf(-(1.0f-fog_density) * (1.0f-fog_density) * (f32)(z * z));
}

void R_DrawFog(){
	if(!fog_density || r_nofog.value) return;
	if(!fog_initialized) R_InitFog();
	sb_updates = 0; // draw sbar over fog
	s32 style = r_fogstyle.value;
	s32 j = 0;
	for(s32 y = scr_vrect.y; y < scr_vrect.y + scr_vrect.height; ++y){
	for(s32 x = scr_vrect.x; x < scr_vrect.x + scr_vrect.width; ++x){
		u8 *pdest = screen->pixels;	
		s32 i = x + y * vid.width;
		s32 bias = randarr[(scr_vrect.width*scr_vrect.height - j)
			% RANDARR_SIZE] * 10 * r_fognoise.value;
		++j;
		f32 ffactor = compute_fog(d_pzbuffer[i]+bias)*r_fogfactor.value;
		switch(style){
		case 0: // noisy
			if((lfsr_random() & 0xFFFF) / 65535.0f < ffactor)
				pdest[i] = fog_pal_index;
			break;
		case 2: // dither + noise
			ffactor -= bias*0.05;
			// fallthrough
		case 1: // dither
			if(D_Dither(&pdest[i], ffactor)) pdest[i]=fog_pal_index;
			break;
		default:
		case 3: // mix
			if(ffactor >= 1) break; 
			((u8*)(pdest))[i] = color_mix_lut[((u8 *)(pdest))[i]]
				[fog_pal_index][(s32)(ffactor*FOG_LUT_LEVELS)];
			break;
	}}}
}
