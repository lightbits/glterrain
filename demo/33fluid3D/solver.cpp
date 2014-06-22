#include "solver.h"
#include <gl/shaderprogram.h>

ShaderProgram
	shader_advect,
	shader_jacobi,
	shader_divergence,
	shader_project;

bool loadShader(ShaderProgram *shader, string path)
{
	string paths[] = { path };
	GLenum types[] = { GL_COMPUTE_SHADER };
	if (!shader->loadFromFile(paths, types, 1))
		return false;
	if (!shader->linkAndCheckStatus())
		return false;
	return true;
}

bool initSolver()
{
	if (
		!loadShader(&shader_advect, "./demo/33fluid3D/advect.cs") ||
		!loadShader(&shader_jacobi, "./demo/33fluid3D/jacobi.cs") ||
		!loadShader(&shader_divergence, "./demo/33fluid3D/divergence.cs") ||
		!loadShader(&shader_project, "./demo/33fluid3D/project.cs")
		)
		return false;
	return true;
}

void freeSolver()
{
	shader_advect.dispose();
	shader_jacobi.dispose();
	shader_divergence.dispose();
	shader_project.dispose();
}

void advect(
	GLuint velocityIn, 
	GLuint quantityIn, 
	GLuint quantityOut)
{
	shader_advect.begin();
	shader_advect.setUniform("textureSize", vec3(GRID_SIZE_X, GRID_SIZE_Y, GRID_SIZE_Z));
	shader_advect.setUniform("dt", TIMESTEP);
	shader_advect.setUniform("VelocityIn", 0);
	shader_advect.setUniform("QuantityIn", 1);
	shader_advect.setUniform("QuantityOut", 2);
	glBindImageTexture(0, velocityIn, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(1, quantityIn, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(2, quantityOut, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glDispatchCompute(NUM_WORK_GROUPS_X, NUM_WORK_GROUPS_Y, NUM_WORK_GROUPS_Z);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	shader_advect.end();
}

void divergence(
	GLuint velocityIn,
	GLuint divergenceOut)
{
	shader_divergence.begin();
	shader_divergence.setUniform("cellSize", 1.0f / vec3(GRID_SIZE_X, GRID_SIZE_Y, GRID_SIZE_Z));
	shader_divergence.setUniform("VelocityIn", 0);
	shader_divergence.setUniform("DivergenceOut", 1);
	glBindImageTexture(0, velocityIn, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(1, divergenceOut, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glDispatchCompute(NUM_WORK_GROUPS_X, NUM_WORK_GROUPS_Y, NUM_WORK_GROUPS_Z);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	shader_divergence.end();
}

void jacobi(
	GLuint divergenceIn,
	GLuint pressureIn,
	GLuint pressureOut)
{
	shader_jacobi.begin();
	shader_jacobi.setUniform("cellSize", 1.0f / vec3(GRID_SIZE_X, GRID_SIZE_Y, GRID_SIZE_Z));
	shader_jacobi.setUniform("DivergenceIn", 0);
	shader_jacobi.setUniform("PressureIn", 1);
	shader_jacobi.setUniform("PressureOut", 2);
	glBindImageTexture(0, divergenceIn, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(1, pressureIn, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(2, pressureOut, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glDispatchCompute(NUM_WORK_GROUPS_X, NUM_WORK_GROUPS_Y, NUM_WORK_GROUPS_Z);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	shader_jacobi.end();
}

void project(
	GLuint pressureIn,
	GLuint velocityOut)
{
	shader_project.begin();
	shader_project.setUniform("cellSize", 1.0f / vec3(GRID_SIZE_X, GRID_SIZE_Y, GRID_SIZE_Z));
	shader_project.setUniform("PressureIn", 0);
	shader_project.setUniform("VelocityOut", 1);
	glBindImageTexture(0, pressureIn, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(1, velocityOut, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glDispatchCompute(NUM_WORK_GROUPS_X, NUM_WORK_GROUPS_Y, NUM_WORK_GROUPS_Z);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	shader_project.end();
}

GLuint createTexture3D(int w, int h, int d, GLenum internalFormat)
{
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_3D, texture);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexImage3D(GL_TEXTURE_3D, 0, internalFormat, w, h, d, 0, GL_RED, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_3D, 0);
}

void Slab::create()
{
	ping = createTexture3D(GRID_SIZE_X, GRID_SIZE_Y, GRID_SIZE_Z, GL_RGBA32F);
	pong = createTexture3D(GRID_SIZE_X, GRID_SIZE_Y, GRID_SIZE_Z, GL_RGBA32F);
}

void Slab::swapSurfaces()
{
	GLuint temp = ping;
	ping = pong;
	pong = ping;
}

void clearSurface(GLuint surface)
{
	glBindTexture(GL_TEXTURE_3D, surface);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	unsigned char *data = new unsigned char[GRID_SIZE_X * GRID_SIZE_Y * GRID_SIZE_Z * 3];

	for (int i = 0; i < GRID_SIZE_X * GRID_SIZE_Y * GRID_SIZE_Z * 3; ++i)
		data[i] = 0;
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, 
		GRID_SIZE_X, GRID_SIZE_Y, GRID_SIZE_Z, 
		0, GL_RGB, GL_UNSIGNED_BYTE, data);

	delete[] data;
}