#ifndef DEFINES_H
#define DEFINES_H

#include <stdint.h>

/*
common primitive types and related constants aliased for readability.
size is in bits
*/


#define DEBUG 1

#if DEBUG
#define Assert(Expression, msg) if(!(Expression)) {printf("Fatal: %s",msg); volatile int* ptr = 0; *ptr = 0;}
#else
#define Assert(Expression)
#endif


#define Kilobytes(Value) ((uint64_t)(Value) * 1024)
#define Megabytes(Value) (Kilobytes(Value) * 1024)
#define Gigabytes(Value) (Megabytes(Value) * 1024)
#define Terabytes(Value) (Gigabytes(Value) * 1024)



typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

/*
Meant to be used as booleans
*/
typedef int8_t b8;

#endif