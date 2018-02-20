// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
#pragma once

#include <cstddef>

struct Vec2
{
  float x, y;

  Vec2() = default;
  Vec2(float x_, float y_) : x(x_), y(y_) {}

  void operator += (Vec2 other) { *this = *this + other; }
  void operator -= (Vec2 other) { *this = *this - other; }
  float operator * (Vec2 other) const { return x * other.x + y * other.y; }

  Vec2 operator + (Vec2 other) const { return Vec2 { x + other.x, y + other.y }; }
  Vec2 operator - (Vec2 other) const { return Vec2 { x - other.x, y - other.y }; }
  Vec2 operator * (float f) const { return Vec2 { x* f, y* f }; }

  static Vec2 zero() { return Vec2(0, 0); }
};

template<typename T>
struct span
{
  size_t len;
  T* ptr;

  T* begin() { return ptr; }
  T* end() { return ptr + len; }
};

struct Segment
{
  Vec2 a, b;
};

enum Shape
{
  Circle,
  Box,
};

// Tries to move a circle of ray 'RAY', initially at 'pos',
// to the position 'pos+delta'.
// Collides with 'segments', and slides along them on collision.
void slideMove(Vec2& pos, Shape shape, Vec2 delta, span<Segment> segments, Vec2& vel);

static auto const RAY = 0.8f;

