SRC="src/*.c"
LIB="-lssl -lcrypto -lcurl -lm"
FLG="-O3 -Wall -Wextra"

clang $SRC -o ellbot $LIB $FLG
