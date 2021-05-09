/// compile-time version of sv_hash (sv.c:144:1) ///

#ifndef TBL_SIZE
#error "`TBL_SIZE` has not been defined"
#endif

#define constexpr_hash(str, len) (__eval(__hash_ ## len, str) & (TBL_SIZE-1))

#define __eval(f, ...) f(__VA_ARGS__)

#define __hash_0(str) (0x1505)
#define __hash_1(str) (str[0] + 33*__hash_0(str))
#define __hash_2(str) (str[1] + 33*__hash_1(str))
#define __hash_3(str) (str[2] + 33*__hash_2(str))
#define __hash_4(str) (str[3] + 33*__hash_3(str))
#define __hash_5(str) (str[4] + 33*__hash_4(str))
#define __hash_6(str) (str[5] + 33*__hash_5(str))
#define __hash_7(str) (str[6] + 33*__hash_6(str))
#define __hash_8(str) (str[7] + 33*__hash_7(str))
