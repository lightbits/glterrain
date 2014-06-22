#ifndef SOLVER_H
#define SOLVER_H
#include <gl/opengl.h>
const int GRID_SIZE_X = 128;
const int GRID_SIZE_Y = 128;
const int GRID_SIZE_Z = 1;
const int LOCAL_GROUP_SIZE_X = 4;
const int LOCAL_GROUP_SIZE_Y = 4;
const int LOCAL_GROUP_SIZE_Z = 1;
const int NUM_WORK_GROUPS_X = GRID_SIZE_X / LOCAL_GROUP_SIZE_X;
const int NUM_WORK_GROUPS_Y = GRID_SIZE_Y / LOCAL_GROUP_SIZE_Y;
const int NUM_WORK_GROUPS_Z = GRID_SIZE_Z / LOCAL_GROUP_SIZE_Z;
const float TIMESTEP = 0.0001f;

struct Slab
{
	GLuint ping;
	GLuint pong;
	void create();
	void swapSurfaces();
};

void clearSurface(GLuint surface);

bool initSolver();
void freeSolver();

/* Transport the quantity through the velocity field. */
void advect(
	GLuint velocityIn, 
	GLuint quantityIn, 
	GLuint quantityOut);

/* Calculate the divergence of the velocity field. */
void divergence(
	GLuint velocityIn,
	GLuint divergenceOut);

/* Solve for the pressure field through jacobi iteration. */
void jacobi(
	GLuint divergenceIn,
	GLuint pressureIn,
	GLuint pressureOut);

/* Calculate the divergence-free component of the velocity field
by subtracting the gradient of the pressure field. */
void project(
	GLuint pressureIn,
	GLuint velocityOut);

#endif