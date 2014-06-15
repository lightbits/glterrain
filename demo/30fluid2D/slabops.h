/* A set of GPU programs that does fluidy things, like
* Advection: Moving quantities around in the fluid
* Jacobi iteration: Solving Poisson equations
* Dissipation: Making things scatter
* Adding forces: You know
*/

#ifndef SLAB_OPS_H
#define SLAB_OPS_H
#include <graphics/renderer.h>
#include <app/glcontext.h>
#include "slab.h"
const int	GRID_SIZE = 256;
const float DX = 1.0f / GRID_SIZE;

bool loadSlabOps();

void jacobi(
	RenderTexture &inputTexture,
	RenderTexture &outputTexture,
	RenderTexture &sourceTexture,
	float alpha, float beta,
	Renderer &gfx, Context &ctx, float dt);

void advect(
	RenderTexture &velocityTexture,
	RenderTexture &sourceTexture,
	RenderTexture &outputTexture,
	float dissipation,
	Renderer &gfx, Context &ctx, float dt);

void addForces(
	RenderTexture &inputTexture,
	RenderTexture &outputTexture,
	vec2 mouse_pos, vec2 mouse_vel,
	Renderer &gfx, Context &ctx, float dt);

void addDye(
	RenderTexture &inputTexture,
	RenderTexture &outputTexture,
	vec2 mouse_pos,
	Renderer &gfx, Context &ctx, float dt);

void calculateDivergence(
	RenderTexture &velocityTexture,
	RenderTexture &outputTexture,
	Renderer &gfx, Context &ctx, float dt);

void subtractGradient(
	RenderTexture &velocityTexture,
	RenderTexture &pressureTexture,
	RenderTexture &outputTexture,
	Renderer &gfx, Context &ctx, float dt);

void applyBoundaryConditions(
	RenderTexture &sourceTexture,
	RenderTexture &outputTexture,
	float factor,
	Renderer &gfx, Context &ctx, float dt);

#endif