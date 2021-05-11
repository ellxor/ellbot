#include "sv.h"

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

        while (sv->count > 0 && sv->mem[0] != delim)
        {
                sv->mem++;
                sv->count--;
        }

        result.count = sv->mem - result.mem;

        if (sv->count > 0 && sv->mem[0] == delim)
        {
                sv->mem++;
                sv->count--;
        }

        return result;
}

void
trim(SV *sv)
{
        while (sv->count > 0 && sv->mem[0] == ' ')
        {
                sv->mem++;
                sv->count--;
        }
}

int
sv_eq(SV a, SV b)
{
        if (a.count != b.count)
        {
                return 0;
        }
        
        for (int i = 0; i < a.count; i++)
        {
                if (a.mem[i] != b.mem[i])
                {
                        return 0;
                }
        }

        return 1;
}

int
sv_cmp(const void *a, const void *b)
{
        SV sa = *(const SV *)a;
        SV sb = *(const SV *)b;

        int len = (sa.count < sb.count)
                ? sa.count
                : sb.count;

        return strncmp(sa.mem, sb.mem, len);
}

int
sv_expect(SV *sv, SV e)
{
        if (e.count > sv->count)
        {
                return 0;
        }

        for (int i = 0; i < e.count; i++)
        {
                if (sv->mem[i] != e.mem[i])
                {
                        return 0;
                }
        }

        sv->mem += e.count;
        sv->count -= e.count;
        return 1;
}

uint32_t
sv_parse_uint(SV sv, bool *ok)
{
        uint32_t res = 0;

        for (int i = 0; i < sv.count; i++)
        {
                char c = sv.mem[i];
                if (!('0' <= c && c <= '9'))
                {
                        *ok = false;
                        return 0;
                }

                res = 10 * res + c - '0';
        }

        *ok = true;
        return res;
}

uint32_t
sv_hash(SV sv)
{
        uint32_t hash = 0x1505;

        for (int i = 0; i < sv.count; i++)
        {
                hash = 33*hash + sv.mem[i];
        }

        return hash;
}
