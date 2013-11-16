#ifndef RENDERER_H
#define RENDERER_H
#include <gl/opengl.h>
#include <gl/bufferedmesh.h>
#include <gl/shaderprogram.h>
#include <graphics/mesh.h>
#include <graphics/spritebatch.h>
#include <graphics/color.h>
#include <iostream>

class Renderer /* GLrenderer : public Renderer */
{
public:
	Renderer();
	void init();
	void dispose();

	/* Enables the checking of pixels with a depth test criterion */
	void enableDepthTest(GLenum depthFunc);
	void disableDepthTest();

	void enableBlending(GLenum srcFactor, GLenum destFactor);
	void disableBlending();

	void enableCulling(GLenum cullFace, GLenum frontFaceOrientation);
	void disableCulling();

	void clearColorBuffer();
	void clearDepthBuffer();
	void clearColorAndDepth();
	void setClearColor(const Color &color);
	void setClearDepth(double depth);

	/* Renders the buffer data using the currently set program */
	void draw(MeshBuffer &mesh, GLenum drawMode);

	/* Renders a set of meshes */
	void draw(std::vector<MeshBuffer> &meshes);

	void beginCustomShader(ShaderProgram &sp);
	void endCustomShader();

	/*void beginDefaultShader();
	void endDefaultShader();*/
	ShaderProgram *getCurrentShaderProgram() { return currentShaderProgram; }
private:
	ShaderProgram *currentShaderProgram;
};

Renderer *getActiveRenderer();
ShaderProgram *getActiveShader();

void setActiveRenderer(Renderer &r);

#endif