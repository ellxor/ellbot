#pragma once
#include <stddef.h>

typedef struct
{
        const char *mem;
        int count;
}
SV;

#define SV(str) ((SV) { .mem = (str), .count = sizeof(str)-1 })
SV sv_from(const char *str, int count);

SV chop(SV *sv, int i);
SV chop_by_delim(SV *sv, char delim);
int expect(SV *sv, SV e);
