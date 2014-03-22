/*
First person camera view logic.
Call fp_update_camera once per update, and set the view matrix
in the shaders as fp_get_view_matrix()
*/

#ifndef FP_CAMERA_H
#define FP_CAMERA_H
#include <app/context.h>
#include <graphics/renderer.h>

void resetCamera(float theta, float phi, vec3 position);
void updateCamera(Renderer &gfx, Context &ctx, double dt);
mat4 getCameraView();

#endif