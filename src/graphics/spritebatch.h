#ifndef SPRITE_BATCH_H
#define SPRITE_BATCH_H
#include <common/rectangle.h>
#include <common/vec.h>
#include <common/matrix.h>
#include <gl/bufferobject.h>
#include <gl/shaderprogram.h>
#include <gl/texture.h>
#include <graphics/renderer.h>
#include <graphics/font.h>
#include <graphics/color.h>
#include <graphics/renderstates.h>

/*
Sprites can be sorted before drawing them. Sorting methods use the sprite's
z-coordinate or texture. Sorting by texture usually gives best performance.
*/
enum class SpriteSortMode { BackToFront, FrontToBack, Texture, None };

struct SpriteInfo
{
	SpriteInfo() : 
		z(1.0f),
		zAxisRotation(0.0f), 
		uLeft(0.0f), uRight(1.0f),
		vBottom(0.0f), vTop(1.0f),
		center(0.0f, 0.0f),
		destination(0.0f, 0.0f, 0.0f, 0.0f), 
		color(1.0f, 1.0f, 1.0f, 1.0f), 
		texture(nullptr) { }
	float z;
	float uLeft, uRight;
	float vBottom, vTop;
	vec2 center;
	Rectanglef destination;
	float zAxisRotation;
	Color color;
	const Texture *texture;
};

/*
Renders textured quads in optimized batches, using an orthographic projection.
*/
class SpriteBatch
{
public:
	static const int SPRITE_COUNT = 512;
	static const int VERTICES_PER_SPRITE = 4;
	static const int VERTEX_SIZE = 9 * sizeof(float);
	static const int INDICES_PER_SPRITE = 6;
	static const int INDEX_SIZE = sizeof(unsigned int); // TODO: Use unsigned short instead?
public:
	SpriteBatch();
	~SpriteBatch();

	void setFont(const Font &font);

	void begin(BlendState blendMode = BlendStates::AlphaBlend, const mat4 &view = mat4(1.0f));
	// void begin(const ShaderProgram &customShader, SpriteBlendMode blendMode = SpriteBlendMode.AlphaBlend, const mat4 &view = mat4(1.0f));

	void drawTexture(const Texture &texture,
					 const Color &color,
					 const Rectanglef &dest,
					 float uLeft, float uRight,
					 float vBottom, float vTop,
					 float depth = 0.0f,
					 float orientation = 0.0f,
					 vec2 center = vec2(0.0f, 0.0f));

	void drawTexture(const Texture &texture, 
					 const Color &color, 
					 const Rectanglef &dest, 
					 const Rectanglei &src, 
					 float depth = 0.0f, 
					 float orientation = 0.0f,
					 vec2 center = vec2(0.0f, 0.0f));

	void drawTexture(const Texture &texture, 
					 const Color &color, 
					 const Rectanglef &dest, 
					 float depth = 0.0f, 
					 float orientation = 0.0f,
					 vec2 center = vec2(0.0f, 0.0f));

	void drawTexture(const Texture &texture, 
					 const Color &color, 
					 const vec2 &pos, 
					 const Rectanglei &src, 
					 float depth = 0.0f, 
					 float orientation = 0.0f,
					 vec2 center = vec2(0.0f, 0.0f));

	void drawTexture(const Texture &texture, 
					 const Color &color, 
					 const vec2 &pos, 
					 float depth = 0.0f, 
					 float orientation = 0.0f,
					 vec2 center = vec2(0.0f, 0.0f));

	 void drawString(const std::string &text, const vec2 &pos, const Color &color);

	/* Draws all buffered sprite data, using the default shader program,
	unless a different one was bound in the process - in which case it uses
	that. */
	void end();
private:
	// Disable copying
	SpriteBatch(const SpriteBatch &copy);
	SpriteBatch &operator=(const SpriteBatch &rhs);

	void sortSprites();

	/* Builds up the vertex and index data buffers with sprite data, and issues a drawcall
	using glDrawElements. */
	void renderBatch(const Texture *batchTexture, SpriteInfo *first, int count);

	/* Adds a single sprite's data to the vertex and index data buffers */
	void renderSprite(const SpriteInfo *sprite);
private:
	BlendState blendState;
	SpriteSortMode sortMode;

	BufferObject vertexBuffer;
	BufferObject indexBuffer;

	mat4 viewMatrix;
	mat4 projectionMatrix;

	const Font *currentFont;
	const Texture *currentTexture;
	ShaderProgram *currentShader;
	ShaderProgram defaultShader;

	std::vector<SpriteInfo> spriteQueue; // Use dynamic array instead?
	bool inBeginEndPair;
};

#endif