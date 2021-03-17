#pragma once
#include <string.h>
#include <stdint.h>

typedef struct
{
        const char *mem;
        int count;
}
SV;

#define SV(str) ((SV) { .mem = (str), .count = sizeof(str)-1 })
#define sv_arg(sv) (sv).count, (sv).mem 

SV sv_from(const char *str, int count);
SV chop(SV *sv, int i);
SV chop_right(SV *sv, int i);
SV chop_by_delim(SV *sv, char delim);
int sv_eq(SV a, SV b);
int sv_expect(SV *sv, SV e);
uint32_t sv_parse_uint(SV sv);
uint32_t sv_hash(SV sv);
