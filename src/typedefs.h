#include <stdint.h>

typedef unsigned char byte; // TODO remove
typedef unsigned char pixel_t; // TODO remove
#ifndef QTYPEDEFS_
#define QTYPEDEFS_
typedef uint8_t  u8;                                           // standard types
typedef int8_t   s8;
typedef uint16_t u16;
typedef int16_t  s16;
typedef uint32_t u32;
typedef int32_t  s32;
typedef uint64_t u64;
typedef int64_t  s64;
typedef float    f32;
typedef double   f64;
typedef struct { f32 l, a, b; } lab_t;                               // rgbtoi.c
typedef struct vrect_s {                                                  // vid
        s32 x,y,width,height; struct vrect_s *pnext;
} vrect_t;
typedef struct {
        u8  *buffer; // invisible buffer
        u8  *colormap; // 256 * VID_GRADES size
        u16 *colormap16; // 256 * VID_GRADES size
        u32 fullbright; // index of first fullbright color
        u32 rowbytes; // may be > width if displayed in a window
        u32 width;
        u32 height;
        f32 aspect; // width / height -- < 0 is taller than wide
        u32 numpages;
        u32 recalc_refdef; // if true, recalc vid-based stuff
        u8  *conbuffer;
        u32 conwidth;
        u32 conheight;
        u32 maxwarpwidth;
        u32 maxwarpheight;
        u8  *direct; // direct drawing to framebuffer, if not NULL
} viddef_t;
#endif
