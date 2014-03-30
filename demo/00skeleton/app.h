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
void update(Renderer &gfx, Context &ctx, double dt);
void render(Renderer &gfx, Context &ctx, double dt);

/* See SDL_KeyboardEvent for values of mod and key */
void keyPressed(int mod, SDL_Keycode key);

/* See SDL_KeyboardEvent for values of mod and key */
void keyReleased(int mod, SDL_Keycode key);

/* See SDL_MouseMotionEvent */
void mouseMoved(int x, int y, int dx, int dy);

/* See SDL_MouseButtonEvent */
void mouseDragged(int button, int x, int y, int dx, int dy);

/* See SDL_MouseButtonEvent */
void mousePressed(int button, int x, int y);

/* See SDL_MouseButtonEvent */
void mouseReleased(int button, int x, int y);

#endif