// Copyright (C) 2018 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

///////////////////////////////////////////////////////////////////////////////
// simulation
#include "collision.h"
#include <vector>
#include <cmath>

using namespace std;

struct Input
{
  bool left, right, up, down;
  bool force;
  bool changeShape;
  bool quit;
};

struct World
{
  Vec2 pos;
  Vec2 rayEnd;
  float angle;
  Shape shape = Circle;

  vector<Segment> segments;
};

Vec2 direction(float angle)
{
  return Vec2(cos(angle), sin(angle));
}

template<size_t N>
void pushPolygon(vector<Segment>& out, Vec2 const (&data)[N])
{
  for(size_t i = 0; i < N; ++i)
  {
    auto a = data[(i + 0) % N];
    auto b = data[(i + 1) % N];
    out.push_back(Segment { a, b });
  }
}

World createWorld()
{
  World world;

  world.pos = Vec2(4, 2);
  world.angle = 0;

  static const Vec2 points1[] =
  {
    Vec2(8, -3),
    Vec2(8, 2),
    Vec2(12, 2),
    Vec2(12, 3),
    Vec2(12, 5),
    Vec2(14, 5),
    Vec2(12, 5),
    Vec2(12, 7),
    Vec2(15, 7),
    Vec2(15, 16),
    Vec2(-3, 16),
    Vec2(-3, 14),
    Vec2(10, 14),
    Vec2(10, 4),
    Vec2(8, 4),
    Vec2(4, 4),
    Vec2(4, 5),
    Vec2(8, 8),
    Vec2(3, 8),
    Vec2(3, 13),
    Vec2(1, 13),
    Vec2(-1, 13),
    Vec2(-3, 13),
    Vec2(-3, 10),
    Vec2(1, 10),
    Vec2(1, 8),
    Vec2(0, 8),
    Vec2(-1, 3.5),
    Vec2(-2, 3.5),
    Vec2(-2, 3.0),
    Vec2(-3, 3.0),
    Vec2(-3, 2.5),
    Vec2(-4, 2.5),
    Vec2(-4, 2),
    Vec2(-5, 2),
    Vec2(-5, -3),
  };

  pushPolygon(world.segments, points1);

  static const Vec2 points2[] =
  {
    Vec2(-1, -2.0),
    Vec2(-2, -2.0),
    Vec2(-1.5, -1),
  };

  pushPolygon(world.segments, points2);

  static const Vec2 points3[] =
  {
    Vec2(2, -3),
    Vec2(3, -2),
    Vec2(4, -3),
    Vec2(3, -2),
    Vec2(4, -1),
    Vec2(5, -2),
    Vec2(6, -3),
  };

  pushPolygon(world.segments, points3);

  static const Vec2 points4[] =
  {
    Vec2(12, 7),
    Vec2(12, 9),
    Vec2(13, 9),
    Vec2(13, 7),
  };

  pushPolygon(world.segments, points4);

  static const Vec2 points5[] =
  {
    Vec2(12, 9),
    Vec2(12, 11),
    Vec2(13, 11),
    Vec2(13, 9),
  };

  pushPolygon(world.segments, points5);
  return world;
}

void tick(World& world, Input input)
{
  float omega = 0;
  float thrust = 0;

  if(input.left)
    omega += 0.1;

  if(input.right)
    omega -= 0.1;

  if(input.down)
    thrust -= 0.08;

  if(input.up)
    thrust += 0.08;

  if(input.changeShape)
    world.shape = Shape(1 - world.shape);

  world.angle += omega;
  auto const delta = direction(world.angle) * thrust;

  auto segments = span<Segment> { world.segments.size(), world.segments.data() };

  if(input.force)
    world.pos += delta;
  else
    slideMove(world.pos, world.shape, delta, segments);

  {
    auto delta = direction(world.angle) * 10;
    auto ratio = raycast(world.pos, world.pos + delta, segments);
    world.rayEnd = world.pos + delta * ratio;
  }
}

///////////////////////////////////////////////////////////////////////////////
// sdl entry point

#include <stdexcept>

#include "SDL.h"
#include "collision.h"

void drawScreen(SDL_Renderer* renderer, World& world)
{
  auto transform = [] (Vec2 v)
    {
      auto const scale = 20.0;
      SDL_Point r;
      r.x = 220 + v.x * scale;
      r.y = 400 - v.y * scale;
      return r;
    };

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);

  for(auto& segment : world.segments)
  {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

    auto a = transform(segment.a);
    auto b = transform(segment.b);

    SDL_RenderDrawLine(renderer, a.x, a.y, b.x, b.y);

    SDL_Rect rect { a.x - 2, a.y - 2, 4, 4 };
    SDL_RenderDrawRect(renderer, &rect);
  }

  {
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);

    {
      auto origin = transform(world.pos);
      auto target = transform(world.pos + direction(world.angle) * RAY);
      SDL_RenderDrawLine(renderer, origin.x, origin.y, target.x, target.y);
    }

    if(world.shape == Circle)
    {
      auto const N = 20;
      auto prev = transform(world.pos);

      for(int i = 0; i <= N; ++i)
      {
        auto a = transform(world.pos + direction(world.angle + i * 2 * M_PI / N) * RAY);
        SDL_RenderDrawLine(renderer, a.x, a.y, prev.x, prev.y);
        prev = a;
      }
    }
    else
    {
      SDL_Point corners[4];
      corners[0] = transform(world.pos + Vec2(-RAY, -RAY));
      corners[1] = transform(world.pos + Vec2(-RAY, +RAY));
      corners[2] = transform(world.pos + Vec2(+RAY, +RAY));
      corners[3] = transform(world.pos + Vec2(+RAY, -RAY));
      corners[4] = corners[0];
      SDL_RenderDrawLines(renderer, corners, 5);
    }
  }

  {
    auto a = transform(world.pos);
    auto b = transform(world.rayEnd);
    SDL_SetRenderDrawColor(renderer, 0, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawLine(renderer, a.x, a.y, b.x, b.y);
  }

  SDL_RenderPresent(renderer);
}

Input readInput()
{
  Input r {};

  SDL_Event event;

  while(SDL_PollEvent(&event))
  {
    if(event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE))
      r.quit = true;

    if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE)
      r.changeShape = true;
  }

  auto keys = SDL_GetKeyboardState(nullptr);
  r.left = keys[SDL_SCANCODE_LEFT];
  r.right = keys[SDL_SCANCODE_RIGHT];
  r.down = keys[SDL_SCANCODE_DOWN];
  r.up = keys[SDL_SCANCODE_UP];
  r.force = keys[SDL_SCANCODE_HOME];

  return r;
}

int main()
{
  if(SDL_Init(SDL_INIT_VIDEO) != 0)
    throw runtime_error("Can't init SDL");

  SDL_Window* window;
  SDL_Renderer* renderer;

  if(SDL_CreateWindowAndRenderer(640, 480, 0, &window, &renderer) != 0)
  {
    SDL_Quit();
    throw runtime_error("Can't create window");
  }

  auto world = createWorld();

  while(1)
  {
    auto const input = readInput();

    if(input.quit)
      break;

    tick(world, input);
    drawScreen(renderer, world);
    SDL_Delay(10);
  }

  SDL_Quit();
  return 0;
}

