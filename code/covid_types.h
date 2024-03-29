/* date = May 18th 2020 5:46 am */

#include <stdint.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

typedef int8 s8;
typedef int8 s08;
typedef int16 s16;
typedef int32 s32;
typedef int64 s64;
typedef bool32 b32;

typedef uint8 u8;
typedef uint8 u08;
typedef uint16 u16;
typedef uint32 u32;
typedef uint64 u64;

typedef real32 r32;
typedef real64 r64;
typedef real32 f32;
typedef real64 f64;

#if !defined(internal) 
#define internal static
#endif
#define local_persist static 
#define global_variable static

// TODO(diego): Put this in compiler options
#define DEBUG 1

#if DEBUG
#define assert(expr) if(!(expr)) {*(volatile int *)0 = 0;}
#else
#define assert(expr)
#endif

#if DEBUG
#define invalid_code_path assert(0 == 1)
#endif

#define array_count(array) (sizeof(array) / sizeof((array)[0]))
