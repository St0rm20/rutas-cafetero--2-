#!/usr/bin/env bash
# Compila los tres programas en C++ (dijkstra, hull, tsp) en build/
set -e
cd "$(dirname "$0")/.."
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
