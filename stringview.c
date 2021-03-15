#include "stringview.h"

SV
sv_from(const char *str, int count)
{
        SV sv = { .mem = str, .count = count };
        return sv;
}

SV
chop(SV *sv, int i)
{
        SV result = sv_from(sv->mem, i);

        sv->mem += i;
        sv->count -= i;

        return result; 
}

SV
chop_by_delim(SV *sv, char delim)
{
        SV result = sv_from(sv->mem, 0);

        while (*sv->mem != delim)
        {
               sv->mem++;
               sv->count--;
        }

        result.count = sv->mem - result.mem;
        sv->mem++;

        return result;
}

int
expect(SV *sv, SV e)
{
        for (int i = 0; i < e.count; i++)
        {
                if (*sv->mem != e.mem[i])
                {
                        return -1;
                }
        }

        return 0;
}
