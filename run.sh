#!/usr/bin/env bash
set -euo pipefail
g++ -Wall -Wextra main.cpp collision.cpp `sdl2-config --cflags --libs` -o collide2d
./collide2d
