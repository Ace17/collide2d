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
  float dist = INFINITY; // distance (circle center, colliding segment)
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
    return CollisionInfo {};

  auto const dist = magnitude(delta);
  auto const N = delta * (1.0 / dist);

  return CollisionInfo { dist, N };
}

static
CollisionInfo collideWithSegments(Vec2 pos, span<Segment> segments)
{
  CollisionInfo earliestCollision;

  for(auto s : segments)
  {
    auto const collision = collideCircleWithSegment(pos, s.a, s.b);

    if(collision.dist < earliestCollision.dist)
      earliestCollision = collision;
  }

  return earliestCollision;
}

void slideMove(Vec2& pos, Vec2 delta, span<Segment> segments)
{
  for(int i = 0; i < 5; ++i)
  {
    auto const collision = collideWithSegments(pos + delta, segments);

    if(collision.dist == INFINITY)
    {
      pos += delta;
      break;
    }

    // fixup position: push the circle out of the segment
    pos += collision.N * (RAY - collision.dist);

    // only keep tangential part of delta
    delta = removeComponentAlong(delta, collision.N);
  }
}

