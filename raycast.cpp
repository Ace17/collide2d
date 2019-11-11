// Copyright (C) 2019 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
#include "collision.h"
#include <cmath>

static float min(float a, float b) { return a < b ? a : b; }
static Vec2 rotateLeft(Vec2 v) { return Vec2(-v.y, v.x); }
static float scalarProduct(Vec2 a, Vec2 b) { return a.x * b.x + a.y * b.y; }
float clamp(float val, float min, float max) { return val < min ? min : (val > max ? max : val); }

static float raycast(const Segment& segment, Vec2 from, Vec2 to)
{
  Vec2 axes[4];

  axes[0] = (to - from);
  axes[1] = rotateLeft(to - from);
  axes[2] = (segment.b - segment.a);
  axes[3] = rotateLeft(segment.b - segment.a);

  float maxSeparation = -1.0 / 0.0;

  for(auto& axis : axes)
  {
    auto sFrom = scalarProduct(axis, from);
    auto sTo = scalarProduct(axis, to);
    auto sA = scalarProduct(axis, segment.a);
    auto sB = scalarProduct(axis, segment.b);

    // ensure 'sFrom <= sTo', so we can later take 'min(sA, sB)'
    if(sFrom > sTo)
    {
      sFrom = -sFrom;
      sTo = -sTo;
      sA = -sA;
      sB = -sB;
    }

    //    sB     sA    sFrom      sTo
    // ---[======]-----[==========]----
    if(sFrom > sA && sFrom > sB)
      return 1.0; // separated by axis

    //   sFrom      sTo    sB     sA
    // --[==========]------[======]--------
    if(sTo < sA && sTo < sB)
      return 1.0; // separated by axis

    auto separation = clamp((min(sA, sB) - sFrom) / (sTo - sFrom), 0, 1);

    if(separation > maxSeparation)
      maxSeparation = separation;
  }

  return maxSeparation;
}

float raycast(Vec2 from, Vec2 to, span<Segment> segments)
{
  float minRatio = 1.0;

  for(auto& segment : segments)
  {
    auto ratio = raycast(segment, from, to);

    if(ratio < minRatio)
      minRatio = ratio;
  }

  return minRatio;
}

