#define EX extern // globals.h only

#define FOG_LUT_LEVELS 32                                            // rgbtoi.c
#define LIT_LUT_RES 64
EX u8	color_mix_lut[256][256][FOG_LUT_LEVELS];
EX u8	lit_lut[LIT_LUT_RES*LIT_LUT_RES*LIT_LUT_RES];
EX u8	lit_lut_initialized;
EX s32	fog_lut_built;
EX f32	gamma_lut[256];
EX s32	color_conv_initialized;
EX lab_t lab_palette[256];

#define RANDARR_SIZE 19937 // prime to reduce unintended patterns     // d_fog.c
EX s32	fog_initialized;
EX u32	lfsr; // non-zero seed
EX f32	fog_density;
EX f32	fog_red;   // CyanBun96: we store the actual RGB values in these,
EX f32	fog_green; // but they get quantized to a single index in the color
EX f32	fog_blue;  // palette before use, stored in fog_pal_index
EX u8	fog_pal_index;
EX f32	randarr[RANDARR_SIZE];

#undef EX
