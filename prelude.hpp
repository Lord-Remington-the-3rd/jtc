#pragma once 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

typedef long i64;
typedef unsigned long u64;
typedef int i32;
typedef unsigned int u32;
typedef short i16;
typedef unsigned short u16;
typedef signed char i8;
typedef unsigned char u8;

typedef float f32;
typedef double f64;

typedef const char * cstring;

typedef void *(*Allocator)(u64);


#include "array.hpp"
#include "map.hpp"
#include "string.hpp"
