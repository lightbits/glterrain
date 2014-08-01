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
const float	AMBIENT	= 0.0f;
const float	SIGMA = 1.0f;
const float	KAPPA = 0.05f;

bool loadSlabOps();

void jacobi(
	RenderTexture &inputTexture,
	RenderTexture &outputTexture,
	RenderTexture &sourceTexture,
	RenderTexture &obstaclesTexture,
	float alpha, float beta,
	Renderer &gfx, Context &ctx, float dt);

void advect(
	RenderTexture &velocityTexture,
	RenderTexture &sourceTexture,
	RenderTexture &outputTexture,
	RenderTexture &obstaclesTexture,
	float dissipation,
	Renderer &gfx, Context &ctx, float dt);

void addBuocancy(
	RenderTexture &velocityTexture,
	RenderTexture &temperatureTexture,
	RenderTexture &outputTexture,
	Renderer &gfx, Context &ctx, float dt);

void applyImpulse(
	RenderTexture &inputTexture,
	RenderTexture &outputTexture,
	vec2 position, float radius,
	float effect,
	Renderer &gfx, Context &ctx, float dt);

void calculateDivergence(
	RenderTexture &velocityTexture,
	RenderTexture &outputTexture,
	RenderTexture &obstaclesTexture,
	Renderer &gfx, Context &ctx, float dt);

void subtractGradient(
	RenderTexture &velocityTexture,
	RenderTexture &pressureTexture,
	RenderTexture &outputTexture,
	RenderTexture &obstaclesTexture,
	Renderer &gfx, Context &ctx, float dt);

#endif