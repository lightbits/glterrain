#ifndef RENDERER_H
#define RENDERER_H
#include <gl/opengl.h>
#include <gl/bufferedmesh.h>
#include <gl/shaderprogram.h>
#include <graphics/mesh.h>
#include <graphics/spritebatch.h>
#include <graphics/color.h>
#include <graphics/renderstates.h>
#include <iostream>

class Renderer /* GLrenderer : public Renderer */
{
public:
	Renderer();
	void init();
	void dispose();

	void setDepthTestState(DepthTestState state);
	void setCullState(CullState state);
	void setRasterizerState(RasterizerState state);
	void setBlendState(BlendState state);
	void enableUserStates();

	void clearColorBuffer();
	void clearDepthBuffer();
	void clearColorAndDepth();
	void setClearColor(const Color &color);
	void setClearColor(float r, float g, float b, float a = 1.0f);
	void setClearDepth(double depth);

	/* Draws the currently bound vertex and index buffer data */
	void drawIndexedGeometry(GLenum drawMode, int indexCount, GLenum indexType);

	/* Renders the buffer data using the currently set program */
	void draw(MeshBuffer &mesh, GLenum drawMode);

	/* Renders a set of meshes */
	void draw(std::vector<MeshBuffer> &meshes);

	/* Draws a single line very slowly */
	void drawLine(const vec3 &v0, const vec3 &v1, const Color &color);

	/* Draws a single line (as a rectangle) very slowly, with a given thickness */
	void drawLine(const vec3 &v0, const vec3 &v1, const Color &color, float thickness);

	/* Draws a set of lines pretty slowly */
	void drawLines(const std::vector<vec3> &lines);

	void beginCustomShader(ShaderProgram &sp);
	void endCustomShader();

	void begin2d();
	void end2d();

	ShaderProgram *getCurrentShaderProgram() { return currentShaderProgram; }
private:
	BlendState blendState;
	CullState cullState;
	RasterizerState rasterizerState;
	DepthTestState depthTestState;
	ShaderProgram *currentShaderProgram;
};

Renderer *getActiveRenderer();
ShaderProgram *getActiveShader();

void setActiveRenderer(Renderer &r);

#endif