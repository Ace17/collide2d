/*
 * Copyright (C) 2018 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#pragma once

#include <vector>

struct Vec2
{
  float x, y;

  Vec2() = default;
  Vec2(float x_, float y_) : x(x_), y(y_) {}

  Vec2 operator * (float f) const
  {
    return Vec2 { x* f, y* f };
  }

  void operator+=(Vec2 other)
  {
    *this = *this + other;
  }

  void operator-=(Vec2 other)
  {
    *this = *this - other;
  }

  float operator * (Vec2 other) const
  {
    return x * other.x + y * other.y;
  }

  Vec2 operator + (Vec2 other) const
  {
    return Vec2 { x + other.x, y + other.y };
  }

  Vec2 operator - (Vec2 other) const
  {
    return Vec2 { x - other.x, y - other.y };
  }
};

template<typename T>
struct span
{
  size_t len;
  T* ptr;

  T* begin()
  {
    return ptr;
  }

  T* end()
  {
    return ptr + len;
  }
};

struct CollisionInfo
{
  float ratio;
  Vec2 N;
};

struct Polygon
{
  std::vector<Vec2> vertices;
};

void slideMove(Vec2& pos, Vec2 delta, span<Polygon> polygons);

static auto const RAY = 0.95;

