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
        if (i > sv->count)
        {
                i = sv->count;
        }

        SV result = sv_from(sv->mem, i);

        sv->mem += i;
        sv->count -= i;

        return result; 
}

SV
chop_right(SV *sv, int i)
{
        if (i > sv->count)
        {
                i = sv->count;
        }
        
        sv->count -= i;

        SV result = sv_from(sv->mem + sv->count, i - sv->count);
        return result;
}

SV
chop_by_delim(SV *sv, char delim)
{
        SV result = sv_from(sv->mem, 0);

        while (*sv->mem != delim)
        {
                if (sv->count == 0)
                {
                        break;
                }

                sv->mem++;
                sv->count--;
        }

        result.count = sv->mem - result.mem;
        sv->mem++;
        sv->count--;

        return result;
}

int
expect(SV *sv, SV e)
{
        if (e.count > sv->count)
        {
                return -1;
        }

        for (int i = 0; i < e.count; i++)
        {
                if (sv->mem[i] != e.mem[i])
                {
                        return -1;
                }
        }

        sv->mem += e.count;
        sv->count -= e.count;
        return 0;
}
