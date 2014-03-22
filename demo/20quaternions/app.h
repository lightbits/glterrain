#ifndef APP_H
#define APP_H
#include <app/glcontext.h>
#include <graphics/renderer.h>

bool load();
void free();
void init(Renderer &gfx, Context &ctx);
void update(Renderer &gfx, Context &ctx, double dt);
void render(Renderer &gfx, Context &ctx, double dt);

//void keyPressed(int key);
//void keyReleased(int key);
//void mouseMoved(int x, int y);
//void mouseDragged(int x, int y, int button);
//void mousePressed(int x, int y, int button);
//void mouseReleased(int x, int y, int button);
//void windowResized(int w, int h);

#endif