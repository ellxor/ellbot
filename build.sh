SRC="src/*.c"
LIB="-lssl -lcrypto -lcurl -lm"
FLG="-O3 -s -Wall -Wextra -std=c2x"

clang $SRC -o ellbot $LIB $FLG
