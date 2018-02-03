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

struct CollisionInfo
{
  float time; // collision time: 0 for immediate blocking, 1 for "no collision"
  Vec2 N; // collision normal. Pointing towards the moving object
};

// returns the point from the segment [s0;s1] which is the closest to 'pos'
static
Vec2 closestPointToSegment(Vec2 pos, Vec2 s0, Vec2 s1)
{
  auto const segmentLength = magnitude(s1 - s0);
  auto const segmentDir = (s1 - s0) * (1.0 / segmentLength);
  auto const relativePos = pos - s0;
  return s0 + segmentDir * clamp(relativePos * segmentDir, 0.0f, segmentLength);
}

static
CollisionInfo collideCircleWithSegment(Vec2 circleCenter, Vec2 s0, Vec2 s1)
{
  auto const closestPointToCircle = closestPointToSegment(circleCenter, s0, s1);

  auto const delta = circleCenter - closestPointToCircle;

  if(delta * delta > RAY * RAY)
    return CollisionInfo { 1, Vec2::zero() };

  auto const N = delta * (1.0 / magnitude(delta));

  return CollisionInfo { 0, N };
}

template<typename Lambda>
void foreachSegment(Polygon const& poly, Lambda onSegment)
{
  auto const K = (int)poly.vertices.size();

  for(int i = 0; i < K; ++i)
  {
    auto const s0 = poly.vertices[(i + 0) % K];
    auto const s1 = poly.vertices[(i + 1) % K];

    onSegment(s0, s1);
  }
}

static
CollisionInfo collideWithPolygons(Vec2 pos, span<Polygon> polygons)
{
  auto earliestCollision = CollisionInfo { 1, Vec2::zero() };

  auto collideWithSegment = [&] (Vec2 s0, Vec2 s1)
    {
      auto const collision = collideCircleWithSegment(pos, s0, s1);

      if(collision.time < earliestCollision.time)
        earliestCollision = collision;
    };

  for(auto& poly : polygons)
    foreachSegment(poly, collideWithSegment);

  return earliestCollision;
}

void slideMove(Vec2& pos, Vec2 delta, span<Polygon> polygons)
{
  for(int i = 0; i < 5; ++i)
  {
    auto const collision = collideWithPolygons(pos + delta, polygons);

    auto const advance = delta * collision.time;
    pos += advance;
    delta -= advance;

    if(collision.time >= 1)
      break;

    // fixup position: slightly repulsive walls
    pos += collision.N * 0.004;

    // only keep tangential part of delta
    delta = removeComponentAlong(delta, collision.N);
  }
}

