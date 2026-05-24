#!/usr/bin/env bash

set -e

MODE="${1:-debug}"

CC=gcc

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
        CFLAGS=("${COMMON_FLAGS[@]}" "${DEBUG_FLAGS[@]}")
        ;;
    release)
        CFLAGS=("${COMMON_FLAGS[@]}" "${RELEASE_FLAGS[@]}")
        ;;
    *)
        echo "Usage: $0 [debug|release]"
        exit 1
        ;;
esac

cd lib && ./build.sh "$MODE" && cd ..

$CC "${CFLAGS[@]}" main.c lib/battleshipslib.a -o battleships

