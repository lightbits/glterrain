#ifndef SPRITEBATCH_H
#define SPRITEBATCH_H
#include <gl/opengl.h>
#include <gl/bufferobject.h>
#include <gl/vertexformat.h>
#include <gl/program.h>
#include <gl/texture.h>
#include <graphics/font.h>
#include <graphics/color.h>
#include <common/vec.h>

const int		SPRITE_BATCH_SPRITE_COUNT	= 512;
const int		SPRITE_BATCH_VERTEX_SIZE	= 9 * sizeof(GLfloat);
const int		SPRITE_BATCH_VBO_SIZE		= SPRITE_BATCH_SPRITE_COUNT * 4 * SPRITE_BATCH_VERTEX_SIZE;
const int		SPRITE_BATCH_IBO_SIZE		= SPRITE_BATCH_SPRITE_COUNT * 6 * sizeof(GLushort);
const GLenum	SPRITE_BATCH_WINDING_ORDER	= GL_CCW;

/*
A really slow and bad spritebatching implementation.
Works kinda ok if you sort stuff by texture, and you don't have too many textures.
*/

class SpriteBatch
{
public:
	SpriteBatch();
	void dispose();

	/*
	This function initializes uniform and attribute location variables. Be sure
	to do this during the initialization of the application.
	The program is assumed to have certain inputs:
		VERTEX SHADER
		in vec3 position;
		in vec4 color;
		in vec2 texel;
		uniform mat4 projection;
		uniform mat4 view;

		FRAGMENT SHADER
		uniform float texBlend;
		uniform sampler2D tex;
	*/
	void create(const Program &program);

	void begin();
	void end();

	// Include depth value?

	void drawTexture(const Texture &texture, const Color &color, float dstX, float dstY, float dstW, float dstH, float ul, float ur, float vb, float vt);
	void drawTexture(const Texture &texture, const Color &color, float dstX, float dstY, float dstW, float dstH, int srcX, int srcY, int srcW, int srcH);
	void drawTexture(const Texture &texture, const Color &color, float dstX, float dstY, float dstW, float dstH);
	void drawQuad(const Color &color, float x, float y, float w, float h);
	void drawString(const std::string &text, float x, float y, const Color &color);

	void setFont(const Font &font);

private:
	void draw(const Texture *texture, const Color &color, const vec3 &v0, const vec3 &v1, const vec3 &v2, const vec3 &v3, 
		float ul, float ur, float vb, float vt);
	void setTexture(const Texture *texture);
	void flush();

private:
	BufferObject vbo;
	BufferObject ibo;
	const Font *currentFont;
	const Texture *currentTexture;
	const Program *currentProgram;

	GLsizei elementIndex; // Each sprite counts 6 elements (indices)
	GLushort vertexIndex; // Implies a maximum of 65536 vertices

	GLint positionAttribLocation;
	GLint colorAttribLocation;
	GLint texCoordAttribLocation;
	GLint projectionUniform;
	GLint viewUniform;
	GLint texBlendUniform;
	GLint textureSamplerUniform;

	mat4 projectionMatrix;
	mat4 viewMatrix;
};

#endif