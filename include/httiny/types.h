#ifndef HTTINY_TYPES_H
#define HTTINY_TYPES_H

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

// Unsigned types.
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

// Signed types.
typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;

typedef struct {
  u8 *data;
  u64 len;
} string;

typedef string string_nullable;
typedef char cstr; // Ignore that its just a char, it's only for making types
                   // easy and pointers to be explicit.
typedef cstr cstr_nullable;

#endif // !HTTINY_TYPES_H
