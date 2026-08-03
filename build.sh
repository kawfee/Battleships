#!/usr/bin/env bash

set -e

MODE="${1:-debug}"


COMMON_FLAGS=(
    -Wall
    -Wextra
    -fshort-enums
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
)

case "$MODE" in
    debug)
        CC=g++
        CFLAGS=("${COMMON_FLAGS[@]}" "${DEBUG_FLAGS[@]}")
        ;;
    release)
        CC=g++
        CFLAGS=("${COMMON_FLAGS[@]}" "${RELEASE_FLAGS[@]}")
        ;;
    *)
        echo "Usage: $0 [debug|release]"
        exit 1
        ;;
esac

cd lib && ./build.sh "$MODE" && cd ..

echo "building battleships..."
$CC "${CFLAGS[@]}" main.cpp lib/battleshipslib.a -o battleships -lm

