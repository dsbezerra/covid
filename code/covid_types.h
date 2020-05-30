/* date = May 18th 2020 5:46 am */

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float real32;
typedef double real64;

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
