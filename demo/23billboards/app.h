#ifndef APP_H
#define APP_H
#include <iostream>
#include <graphics/renderer.h>
#include <app/glcontext.h>
#include <common/typedefs.h>

bool load();
void free();
void init(Renderer &gfx, Context &ctx);
void update(Renderer &gfx, Context &ctx, double dt);
void render(Renderer &gfx, Context &ctx, double dt);

#endif