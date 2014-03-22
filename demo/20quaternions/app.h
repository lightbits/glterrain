#ifndef APP_H
#define APP_H
#include <app/glcontext.h>
#include <graphics/renderer.h>

bool load();
void free();
void init(Renderer &gfx, Context &ctx);
void update(Renderer &gfx, Context &ctx, double dt);
void render(Renderer &gfx, Context &ctx, double dt);

///* See SDL_KeyboardEvent */
//void keyPressed(int mod, int key);
//
///* See SDL_KeyboardEvent */
//void keyReleased(int mod, int key);
//
///* See SDL_MouseButtonEvent */
//void mouseMoved(int x, int y, int dx, int dy);
//
///* See SDL_MouseButtonEvent */
//void mouseDragged(int button, int x, int y, int dx, int dy);
//
///* See SDL_MouseButtonEvent */
//void mousePressed(int button, int x, int y);
//
///* See SDL_MouseButtonEvent */
//void mouseReleased(int button, int x, int y);

#endif