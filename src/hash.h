/// compile-time version of sv_hash (sv.c:144:1) ///
#pragma once

#ifndef TBL_SIZE
#error `TBL_SIZE` has not been defined
#endif

#define constexpr_hash(str) (xorshift(((long)str)) & (TBL_SIZE-1))

#define shift1(x) (x ^ (x << 13))
#define shift2(x) (x ^ (x >> 17))
#define shift3(x) (x ^ (x <<  5))

#define xorshift(x) shift3(shift2(shift1(x)))
