/*
 * Copyright (C) 2018 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include "collision.h"
#include <cmath>

using namespace std;

template<typename T>
T clamp(T val, T min, T max)
{
  if(val < min)
    return min;
  if(val > max)
    return max;
  return val;
}

static
float magnitude(Vec2 v)
{
  return sqrt(v * v);
}

static
Vec2 removeComponentAlong(Vec2 v, Vec2 u)
{
  auto const comp = v * u;
  return v - u * comp;
}

static
CollisionInfo collideCircleWithSegment(Vec2 circleCenter, Vec2 w0, Vec2 w1)
{
  auto const seg_len = magnitude(w1 - w0);
  auto const seg_v = (w1 - w0) * (1.0 / seg_len);
  auto const rel_b = circleCenter - w0;
  auto const closestPointToCircle = w0 + seg_v * clamp(rel_b * seg_v, 0.0f, seg_len);

  auto const delta = circleCenter - closestPointToCircle;

  if(delta * delta > RAY * RAY)
    return CollisionInfo{1, Vec2(0, 0)};

  auto const N = delta * (1.0 / magnitude(delta));

  return CollisionInfo{0, N};
}

static
CollisionInfo collideWithPolygons(Vec2 pos, span<Polygon> polygons)
{
  auto bestTr = CollisionInfo { 1, Vec2(0, 0) };

  for(auto& poly : polygons)
  {
    auto const K = (int)poly.vertices.size();

    for(int i = 0; i < K; ++i)
    {
      auto const w0 = poly.vertices[(i + 0) % K];
      auto const w1 = poly.vertices[(i + 1) % K];

      auto const tr = collideCircleWithSegment(pos, w0, w1);

      if(tr.ratio < bestTr.ratio)
        bestTr = tr;
    }
  }

  return bestTr;
}

void slideMove(Vec2& pos, Vec2 delta, span<Polygon> polygons)
{
  for(int i = 0; i < 5; ++i)
  {
    auto const tr = collideWithPolygons(pos + delta, polygons);

    auto const advance = delta * tr.ratio;
    pos += advance;
    delta -= advance;

    if(tr.ratio >= 1)
      break;

    // fixup position: slightly repulsive walls
    pos += tr.N * 0.004;

    // keep tangential part of delta
    delta = removeComponentAlong(delta, tr.N);
  }
}

