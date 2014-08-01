#ifndef APP_H
#define APP_H
#include <iostream>
#include <graphics/renderer.h>
#include <app/glcontext.h>
using std::cout;
using std::endl;
using std::vector;
using std::string;

bool load();
void free();
void init(Renderer &gfx, Context &ctx);
void update(Renderer &gfx, Context &ctx, float dt);
void render(Renderer &gfx, Context &ctx, float dt);

void keyPressed(int mod, SDL_Keycode key);

#endif