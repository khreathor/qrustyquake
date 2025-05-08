#include <stdint.h>

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
#endif
