// class ShaderProgram
// {
// 	...
// 	mat4 Model;
// 	mat4 View;
// 	mat4 Projection;
// };

#ifndef SPRITE_BATCH_H
#define SPRITE_BATCH_H
#include <gl/bufferobject.h>
#include <gl/shaderprogram.h>
#include <gl/texture.h>
#include <graphics/renderer.h>
#include <graphics/font.h>
#include <graphics/color.h>
#include <graphics/renderstates.h>

public enum class SpriteBlendMode
{
	Additive, AlphaBlend, None
};

class SpriteBatch
{
public:
	static const int SpriteCount = 512;
	static const int SizeOfSpriteVertex = 9 * sizeof(GLfloat);
	static const int VerticesPerSprite = 4;
	static const int IndicesPerSprite = 6;
	static const int MaximumVboSize = SpriteCount * VerticesPerSprite;
	static const GLenum SpriteWindingOrder = GL_CCW;
public:
	SpriteBatch();
	void create();
	void dispose();

	void begin();
	void begin(const BlendState )
	void begin();
	void begin(const mat4 &transformMatrix);

	/* Draws all buffered sprite data, using the default shader program,
	unless a different one was bound in the process - in which case it uses
	that. */
	void end();
private:
	ShaderProgram defaultShader;
};

#endif