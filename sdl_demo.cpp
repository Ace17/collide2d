/*
 * Copyright (C) 2018 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#include <stdexcept>
#include <iostream>
#include "SDL.h"

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// simulation
#include "collision.h"

struct World
{
  Vec2 pos;
  float angle;

  vector<Polygon> sectors;
};

Vec2 direction(float angle)
{
  return Vec2(cos(angle), sin(angle));
}

void init(World& world)
{
  world.pos = Vec2(4, 2);
  world.angle = 0;

  Polygon sector;

  {
    static const Vec2 points[] =
    {
      Vec2(8, -3),
      Vec2(8, 2),
      Vec2(12, 2),
      Vec2(12, 4),
      Vec2(12, 8),
      Vec2(14, 8),
      Vec2(12, 8),
      Vec2(12, 16),
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

    sector.vertices.assign(begin(points), end(points));
    world.sectors.push_back(sector);
  }

  {
    static const Vec2 points[] =
    {
      Vec2(-1, -2.0),
      Vec2(-2, -2.0),
      Vec2(-1.5, -1),
    };

    sector.vertices.assign(begin(points), end(points));
    world.sectors.push_back(sector);
  }

  {
    static const Vec2 points[] =
    {
      Vec2(6, -2.9),
      Vec2(3, -2.9),
      Vec2(4.5, -2.0),
    };

    sector.vertices.assign(begin(points), end(points));
    world.sectors.push_back(sector);
  }
}

void tick(World& world)
{
  float omega = 0;
  float thrust = 0;
  auto keys = SDL_GetKeyboardState(nullptr);

  if(keys[SDL_SCANCODE_LEFT])
    omega += 0.1;

  if(keys[SDL_SCANCODE_RIGHT])
    omega -= 0.1;

  if(keys[SDL_SCANCODE_DOWN])
    thrust -= 0.08;

  if(keys[SDL_SCANCODE_UP])
    thrust += 0.08;

  if(keys[SDL_SCANCODE_HOME])
    world.pos += direction(world.angle) * 0.1;

  world.angle += omega;
  auto const delta = direction(world.angle) * thrust;

  auto polygons = span<Polygon> { world.sectors.size(), world.sectors.data() };

  for(int i=0;i < 4;++i)
    slideMove(world.pos, delta * 0.25, polygons);
}

///////////////////////////////////////////////////////////////////////////////
// sdl entry point

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

  for(auto& sector : world.sectors)
  {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);

    vector<SDL_Point> sdlPoints;

    for(auto p : sector.vertices)
      sdlPoints.push_back(transform(p));

    sdlPoints.push_back(transform(sector.vertices[0]));

    SDL_RenderDrawLines(renderer, sdlPoints.data(), sdlPoints.size());
  }

  {
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
    vector<SDL_Point> sdlPoints;
    sdlPoints.push_back(transform(world.pos));

    for(int i = 0; i <= 100; ++i)
      sdlPoints.push_back(transform(world.pos + direction(world.angle + i * 2 * M_PI / 100)));

    SDL_RenderDrawLines(renderer, sdlPoints.data(), sdlPoints.size());
  }

  SDL_RenderPresent(renderer);
}

void safeMain()
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

  World world;

  init(world);

  bool keepGoing = true;

  while(keepGoing)
  {
    SDL_Event event;

    while(SDL_PollEvent(&event))
    {
      if(event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE))
        keepGoing = false;
    }

    tick(world);
    drawScreen(renderer, world);
    SDL_Delay(10);
  }

  SDL_Quit();
}

int main()
{
  try
  {
    safeMain();
    return 0;
  }
  catch(const exception& e)
  {
    cerr << "Fatal: " << e.what() << endl;
    return 1;
  }
}

