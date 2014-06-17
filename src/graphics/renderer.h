#ifndef RENDERER_H
#define RENDERER_H
#include <app/context.h>
#include <common/typedefs.h>
#include <gl/opengl.h>
#include <gl/meshbuffer.h>
#include <gl/shaderprogram.h>
#include <gl/framebuffer.h>
#include <graphics/model.h>
#include <graphics/mesh.h>
#include <graphics/spritebatch.h>
#include <graphics/color.h>
#include <graphics/rendertexture.h>
#include <graphics/renderstates.h>
#include <iostream>

class Renderer /* GLrenderer : public Renderer */
{
public:
	Renderer();
	~Renderer();
	void init(Context &ctx);
	void dispose();

	void setDepthTestState(DepthTestState state);
	void setCullState(CullState state);
	void setRasterizerState(RasterizerState state);
	void setBlendState(BlendState state);
	void enableUserStates();

	// void setTextureUnit(int unit);

	void clearColorBuffer();
	void clearDepthBuffer();
	void clearColorAndDepth();
	void clear(uint32 color, double depth);
	void setClearColor(const Color &color);
	void setClearColor(float r, float g, float b, float a = 1.0f);
	void setClearDepth(double depth);
	void setViewport(int x, int y, int w, int h);

	/* Draws the currently bound vertex buffer */
	void drawVertexBuffer(GLenum drawMode, int indexCount);

	/* Draws the currently bound vertex and index buffer data */
	void drawIndexedVertexBuffer(GLenum drawMode, int indexCount, GLenum indexType);

	/* Allocates a buffer for the vertex data in the mesh and renders */
	void draw(Mesh &mesh, GLenum drawMode);

	/* Renders the buffer data using the currently set program */
	void draw(MeshBuffer &mesh, GLenum drawMode);

	/* Renders a set of meshes */
	void draw(std::vector<MeshBuffer> &meshes);

	/* Draws a single model with an associated model transformation */
	void draw(Model &model);

	/* Draws a single line very slowly. Warning: might cause stuttering. */
	void drawLine(const vec3 &v0, const vec3 &v1, const Color &color);

	/* Draws a single line (as a rectangle) very slowly, with a given thickness. Warning: might cause stuttering. */
	void drawLine(const vec3 &v0, const vec3 &v1, const Color &color, float thickness);

	/* Draws a set of lines pretty slowly. Warning: might cause stuttering. */
	void drawLines(const std::vector<vec3> &lines);

	/* Drawss a single flat quad very slowly. Warning: might cause stuttering. */
	void drawQuad(float x, float y, float w, float h);

	/* Drawss a single flat quad, with depth, very slowly. Warning: might cause stuttering. */
	void drawQuad(float x, float y, float w, float h, float z);

	/* Draws a single flat textured quad very slowly. Warning: might cause stuttering. */
	void drawTexQuad(float x, float y, float w, float h);

	/* Sets the current shader to be used while rendering */
	void beginCustomShader(ShaderProgram &sp);

	/* 
	The following functions all work on the current shader program.
	If there is none, an execution is thrown. 
	*/
	GLint getUniformLocation(const std::string &name);
	GLint getAttributeLocation(const std::string &name);
	void bindAttribute(GLuint location, const std::string &name);
	void setAttributefv(const std::string &name, GLsizei numComponents, GLsizei stride, GLsizei offset);
	void setAttributefv(GLint location, GLsizei numComponents, GLsizei stride, GLsizei offset);
	void setAttributeDivisor(const std::string &name, GLuint divisor);
	void unsetAttribute(const std::string &name);
	void setUniform(const std::string &name, const mat4 &mat);
	void setUniform(const std::string &name, const mat3 &mat);
	void setUniform(const std::string &name, const mat2 &mat);
	void setUniform(const std::string &name, const Color &color);
	void setUniform(const std::string &name, const vec4 &vec);
	void setUniform(const std::string &name, const vec3 &vec);
	void setUniform(const std::string &name, const vec2 &vec);
	void setUniform(const std::string &name, GLdouble d);
	void setUniform(const std::string &name, GLfloat f);
	void setUniform(const std::string &name, GLint i);

	/* Unsets the current shader */
	void endCustomShader();

	ShaderProgram *getCurrentShaderProgram() { return currentShaderProgram; }
private:
	BlendState blendState;
	CullState cullState;
	RasterizerState rasterizerState;
	DepthTestState depthTestState;
	ShaderProgram *currentShaderProgram;
};

// HEED CAUTION, FOR THERE BE DRAGONS!
Renderer *getActiveRenderer();

// HEED CAUTION, FOR THERE BE DRAGONS!
// Allowing anything to handle the raw pointer to the current shader is dangerous
// When can it be modified ooh I don't know ANY TIME AT ALL WOW!
// It returns a garbage value? Have fun debugging when you realize that any place
// in your code could have modified it!
ShaderProgram *getActiveShader();

// TODO: Not this
void setActiveRenderer(Renderer &r);

#endif