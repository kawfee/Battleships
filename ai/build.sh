#!/usr/bin/env bash

set -e

MODE="${1:-debug}"

COMMON_FLAGS=(
    -Wall
    -Wextra
    -pedantic
)

DEBUG_FLAGS=(
    -O0
    -g
)

RELEASE_FLAGS=(
    -Werror
    -O3
    -march=native
)

case "$MODE" in
    debug)
        CXX=g++
        CXXFLAGS=("${COMMON_FLAGS[@]}" "${DEBUG_FLAGS[@]}")
        ;;
    release)
        CXX=g++
        CXXFLAGS=("${COMMON_FLAGS[@]}" "${RELEASE_FLAGS[@]}")
        ;;
    *)
        echo "Usage: $0 [debug|release]"
        exit 1
        ;;
esac

SRC="Player.cpp PlayerV2.cpp"

for file in $SRC; do
    obj="${file%.cpp}.o"
    echo "Compiling $file into $obj..."
    $CXX "${CXXFLAGS[@]}" -o "$obj" -c "$file"
done

