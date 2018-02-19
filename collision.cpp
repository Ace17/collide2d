// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
#include "collision.h"
#include <cmath>

static float magnitude(Vec2 v) { return sqrt(v * v); }
static float max(float a, float b) { return a > b ? a : b; }
static float min(float a, float b) { return a < b ? a : b; }
static float abs(float a) { return a >= 0 ? a : -a; }
static Vec2 normalize(Vec2 v) { return v * (1.0 / magnitude(v)); }
static Vec2 rotateLeft(Vec2 v) { return Vec2(-v.y, v.x); }

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

static Collision collideBoxWithSegment(Vec2 center, Segment seg)
{
  const float THICKNESS = 0.1;

  auto const boxHalfSize = Vec2(RAY, RAY);

  Vec2 axes[8];
  int axeCount = 0;

  axes[axeCount++] = Vec2(1, 0); // X axis
  axes[axeCount++] = Vec2(0, 1); // Y axis
  axes[axeCount++] = rotateLeft(normalize(seg.b - seg.a)); // segment normal

  // vector going from seg.b to box center
  if((center - seg.b) * (seg.b - seg.a) > 0)
    axes[axeCount++] = normalize(center - seg.b);

  // vector going from seg.a to box center
  if((center - seg.a) * (seg.a - seg.b) > 0)
    axes[axeCount++] = normalize(center - seg.a);

  auto r = Collision{1.0f/0.0f, Vec2{}};

  for(int i=0;i < axeCount;++i)
  {
    auto const N = axes[i];

    // project the box on the separating axis N
    auto const boxPos = center * N;
    auto const boxRadius = abs(boxHalfSize.x * N.x) + abs(boxHalfSize.y * N.y);
    auto const boxPosMax = boxPos + boxRadius;
    auto const boxPosMin = boxPos - boxRadius;

    // project the segment on the separating axis N
    auto const segPos0 = seg.a * N;
    auto const segPos1 = seg.b * N;
    auto const segPosMax = max(segPos0, segPos1) + THICKNESS;
    auto const segPosMin = min(segPos0, segPos1) - THICKNESS;

    if(boxPosMin > segPosMax || boxPosMax < segPosMin)
      return Collision{}; // box and segments are separated by N

    Collision c;

    if(boxPos > segPos0)
      c = Collision{segPosMax - boxPosMin, N};
    else
      c = Collision{boxPosMax - segPosMin, N * -1.0};

    if(c.depth < r.depth)
      r = c;
  }

  return r;
}

static Collision collideWithSegments(Vec2 pos, Shape shape, span<Segment> segments)
{
  Collision earliestCollision;

  for(auto seg : segments)
  {
    auto collide = shape == Circle ? collideCircleWithSegment : collideBoxWithSegment;
    auto const collision = collide(pos, seg);

    if(collision.depth > earliestCollision.depth)
      earliestCollision = collision;
  }

  return earliestCollision;
}

// discrete collision detection
void slideMove(Vec2& pos, Shape shape, Vec2 delta, span<Segment> segments)
{
  // move to new position ...
  pos += delta;

  // ... then fix it, if needed
  for(int i = 0; i < 5; ++i)
  {
    auto const collision = collideWithSegments(pos, shape, segments);

    if(collision.depth == 0)
      break;

    // fixup position: push the circle out of the segment
    pos += collision.N * collision.depth;
  }
}

