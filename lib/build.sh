#!/usr/bin/env bash

set -e

MODE="${1:-debug}"

BUILD_DIR="build"

YYJSON_DIR="vendor/yyjson/src"

COMMON_FLAGS=(
    -std=c99
    -Wall
    -Wextra
    -pedantic
)

DEBUG_FLAGS=(
    -O0
    -g
    -fsanitize=address
    -fsanitize=undefined
)

RELEASE_FLAGS=(
    -Werror
    -O3
    -march=native
    -flto
)

case "$MODE" in
    debug)
        CC=gcc
        CFLAGS=("${COMMON_FLAGS[@]}" "${DEBUG_FLAGS[@]}")
        YYJSON_OBJ="$BUILD_DIR/yyjson_debug.o"
        ;;
    release)
        CC=clang
        CFLAGS=("${COMMON_FLAGS[@]}" "${RELEASE_FLAGS[@]}")
        YYJSON_OBJ="$BUILD_DIR/yyjson_release.o"
        ;;
    *)
        echo "Usage: $0 [debug|release]"
        exit 1
        ;;
esac

# NOTE(mattg): Because yyjson is slow to compile (~10s), just compile it if the object isn't there
if [ ! -f "$YYJSON_OBJ" ]; then
    echo "Compiling yyjson.c $MODE object..."
    $CC "${CFLAGS[0]}" \
        -DYYJSON_DISABLE_INC_READER \
        -DYYJSON_DISABLE_UTILS \
        -DYYJSON_DISABLE_NON_STANDARD \
        -DYYJSON_DISABLE_UTF8_VALIDATION \
        -c "$YYJSON_DIR/yyjson.c" \
        -o "$YYJSON_OBJ"
fi

SRC="platforms/unix.c runtime.c"
OBJ="$YYJSON_OBJ"


for file in $SRC; do
    obj="build/${file%.c}.o"
    mkdir -p "$(dirname "$obj")"

    echo "Compiling $file into $obj..."
    $CC "${CFLAGS[@]}" -c "$file" -o "$obj"
    OBJ="$OBJ $obj"
done

echo "archiving ${OBJ} into battleshipslib.a..."
ar rcs battleshipslib.a $OBJ
