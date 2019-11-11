#!/usr/bin/env bash
set -euo pipefail
g++ -Wall -Wextra sdl_demo.cpp collision.cpp raycast.cpp -s `sdl2-config --cflags --libs` -o collide2d
./collide2d
