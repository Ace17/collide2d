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

struct Collision
{
  float depth = 0; // distance (circle center, colliding segment)
  Vec2 N; // collision normal. Pointing towards the moving object
};

// returns the point from the segment 'seg' which is the closest to 'pos'
static
Vec2 closestPointOnSegment(Vec2 pos, Segment seg)
{
  auto const segmentLength = magnitude(seg.b - seg.a);
  auto const segmentDir = (seg.b - seg.a) * (1.0 / segmentLength);
  auto const relativePos = pos - seg.a;
  return seg.a + segmentDir * clamp(relativePos * segmentDir, 0.0f, segmentLength);
}

static
Collision collideCircleWithSegment(Vec2 circleCenter, Segment seg)
{
  auto const delta = circleCenter - closestPointOnSegment(circleCenter, seg);

  if(delta * delta > RAY * RAY)
    return Collision {};

  auto const dist = magnitude(delta);
  auto const N = delta * (1.0 / dist);

  return Collision { RAY - dist, N };
}

static
Collision collideWithSegments(Vec2 pos, span<Segment> segments)
{
  Collision earliestCollision;

  for(auto seg : segments)
  {
    auto const collision = collideCircleWithSegment(pos, seg);

    if(collision.depth > earliestCollision.depth)
      earliestCollision = collision;
  }

  return earliestCollision;
}

// discrete collision detection
void slideMove(Vec2& pos, Vec2 delta, span<Segment> segments)
{
  // move to new position ...
  pos += delta;

  // ... then fix it, if needed
  for(int i = 0; i < 5; ++i)
  {
    auto const collision = collideWithSegments(pos, segments);

    if(collision.depth == 0)
      break;

    // fixup position: push the circle out of the segment
    pos += collision.N * collision.depth;
  }
}

