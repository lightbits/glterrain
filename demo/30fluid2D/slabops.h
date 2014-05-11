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
	Renderer &gfx, Context &ctx, float dt);

void addForces(
	RenderTexture &velocityTexture,
	RenderTexture &outputTexture,
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

void applyBoundaryCondition(
	RenderTexture &sourceTexture,
	RenderTexture &outputTexture,
	BufferObject &vbo,
	const vec2 &offset,
	float scale,
	Renderer &gfx, Context &ctx, float dt);

#endif