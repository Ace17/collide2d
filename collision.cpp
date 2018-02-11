// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
#include "collision.h"
#include <cmath>

static float magnitude(Vec2 v) { return sqrt(v * v); }

struct Collision
{
  float depth = 0; // distance (circle center, colliding segment)
  Vec2 N; // collision normal. Pointing towards the moving object
};

// returns the point from the segment 'seg' which is the closest to 'pos'
static Vec2 closestPointOnSegment(Vec2 pos, Segment seg)
{
  auto const tangent = seg.b - seg.a;

  if((pos - seg.a) * tangent <= 0)
    return seg.a; // 'pos' is before 'a' on the line (ab)

  if((pos - seg.b) * tangent >= 0)
    return seg.b; // 'pos' is after 'b' on the line (ab)

  // normalize tangent
  auto const T = tangent * (1.0 / magnitude(tangent));
  auto const relativePos = pos - seg.a;
  return seg.a + T * (T * relativePos);
}

static Collision collideCircleWithSegment(Vec2 circleCenter, Segment seg)
{
  auto const delta = circleCenter - closestPointOnSegment(circleCenter, seg);

  if(delta * delta > RAY * RAY)
    return Collision {};

  auto const dist = magnitude(delta);
  auto const N = delta * (1.0 / dist);
  return Collision { RAY - dist, N };
}

static Collision collideWithSegments(Vec2 pos, span<Segment> segments)
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

