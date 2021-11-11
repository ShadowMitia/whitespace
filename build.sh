#!/usr/bin/bash

MODE=$1
OUTPUT=$2
if [ "${MODE,,}" = "debug" ]; then
    MODE="debug"
elif [ "${MODE,,}" = "release" ]; then
    MODE="release"
else
    MODE="debug"
    echo "Unknown mod defaulting to" $MODE
fi

COMPILER=g++

STANDARD=-std=c++20
WARNINGS=(-Wall -Wextra -Wdisabled-optimization -Wno-unknown-pragmas -Wno-deprecated-copy -Wshadow=compatible-local -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wpedantic -Wconversion -Wsign-conversion -Wnull-dereference -Wdouble-promotion -Wformat=2)
SANITIZERS=(-fsanitize="address,leak,undefined" -fsanitize-address-use-after-scope)
DEBUG=(-Og -g3 -ggdb -fstack-protector -ftrapv -fno-omit-frame-pointer)
OTHER=(-fopenmp)
OPTIMISE=(-foptimize-sibling-calls -faligned-new)
RELEASE=(-O3)

if [ $MODE = "debug" ]; then
    $COMPILER -Wl,-O1 $STANDARD "${DEBUG[@]}" "${WARNINGS[@]}"  "${SANITIZERS[@]}" "${OTHER[@]}" src/main.cpp ${OUTPUT:+ -o "$OUTPUT"}
elif [ "${MODE,,}" = "release" ]; then
    $COMPILER -Wl,-O1 $STANDARD "${RELEASE[@]}" "${OPTIMISE[@]}" "${WARNINGS[@]}" "${OTHER[@]}" src/main.cpp ${OUTPUT:+ -o "$OUTPUT"}
fi
