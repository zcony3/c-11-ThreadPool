#!/usr/bin/env bash

BIN_DIR=bin
BUILD_DIR=build
rm -rf "$BUILD_DIR"

if [ "$1" = "clean" ]; then
rm -rf "$BUILD_DIR"
exit 0
fi

echo "Gonna make with ..."

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake $CMAKE_OPTS \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_VERBOSE_MAKEFILE=ON \../

make 
