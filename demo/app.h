#ifndef APP_H
#define APP_H
#include <app/context.h>

bool loadApp(Context &ctx);
void initApp(Context &ctx);
void updateApp(Context &ctx, double dt);
void renderApp(Context &ctx, double dt);
void freeApp(Context &ctx);

#endif