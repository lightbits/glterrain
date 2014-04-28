#ifndef SPRITE_BATCH_H
#define SPRITE_BATCH_H
#include <common/rectangle.h>
#include <common/matrix.h>
#include <gl/bufferobject.h>
#include <gl/shaderprogram.h>
#include <gl/texture.h>
#include <graphics/font.h>
#include <graphics/color.h>
#include <graphics/renderstates.h>

/*
Sprites can be sorted before drawing them. Sorting methods use the sprite's
z-coordinate or texture. Sorting by texture usually gives best performance.
*/
enum SpriteSortMode { 
	SpriteSortMode_BackToFront, 
	SpriteSortMode_FrontToBack, 
	SpriteSortMode_Texture, 
	SpriteSortMode_None 
};

struct SpriteInfo
	{
		SpriteInfo() : 
			z(1.0f),
			zAxisRotation(0.0f), 
			uLeft(0.0f), uRight(1.0f),
			vBottom(0.0f), vTop(1.0f),
			scale(1.0f),
			center(0.0f, 0.0f),
			destination(0.0f, 0.0f, 0.0f, 0.0f), 
			color(1.0f, 1.0f, 1.0f, 1.0f), 
			texture(nullptr) { }
		float z;
		float uLeft, uRight;
		float vBottom, vTop;
		float scale;
		vec2 center;
		Rectanglef destination;
		float zAxisRotation;
		Color color;
		const Texture2D *texture;
	};

/*
Renders textured quads in optimized batches, using an orthographic projection.
*/
class SpriteBatch
{
public:
	struct Vertex
	{
		Vertex() : x(0), y(0), z(0), r(0), g(0), b(0), a(0), u(0), v(0), s(0) { }
		Vertex(float X, float Y, float Z, float R, float G, float B, float A, float U, float V, float S) : 
			x(X), y(Y), z(Z), r(R), g(G), b(B), a(A), u(U), v(V), s(S) { }
		float x, y, z; // Window position and depth
		float r, g, b, a; // Color
		float u, v; // Texel
		float s; // Scale
	};

	static const int SPRITE_COUNT = 512;
	static const int VERTICES_PER_SPRITE = 4;
	static const int VERTEX_SIZE = 10 * sizeof(float);
	static const int INDICES_PER_SPRITE = 6;
	static const int INDEX_SIZE = sizeof(unsigned int); // TODO: Use unsigned short instead?
public:
	SpriteBatch();
	~SpriteBatch();

	void create(/* int spriteCount = 512 */);
	void dispose();

	void setFont(const Font &font);

	void begin(BlendState blendMode = BlendStates::AlphaBlend, const mat4 &view = mat4(1.0f));
	// void begin(const ShaderProgram &customShader, SpriteBlendMode blendMode = SpriteBlendMode.AlphaBlend, const mat4 &view = mat4(1.0f));

	void drawTexture(const Texture2D &texture,
					 const Color &color,
					 const Rectanglef &dest,
					 float uLeft, float uRight,
					 float vBottom, float vTop,
					 float scale = 1.0f,
					 float depth = 0.0f,
					 float orientation = 0.0f,
					 vec2 center = vec2(0.0f, 0.0f));

	void drawTexture(const Texture2D &texture, 
					 const Color &color, 
					 const Rectanglef &dest, 
					 const Rectanglei &src, 
					 float scale = 1.0f,
					 float depth = 0.0f, 
					 float orientation = 0.0f,
					 vec2 center = vec2(0.0f, 0.0f));

	void drawTexture(const Texture2D &texture, 
					 const Color &color, 
					 const Rectanglef &dest,  
					 float scale = 1.0f,
					 float depth = 0.0f, 
					 float orientation = 0.0f,
					 vec2 center = vec2(0.0f, 0.0f));

	void drawTexture(const Texture2D &texture, 
					 const Color &color, 
					 const vec2 &pos, 
					 const Rectanglei &src,    
					 float scale = 1.0f,
					 float depth = 0.0f, 
					 float orientation = 0.0f,
					 vec2 center = vec2(0.0f, 0.0f));

	void drawTexture(const Texture2D &texture, 
					 const Color &color, 
					 const vec2 &pos, 	  
					 float scale = 1.0f,
					 float depth = 0.0f, 
					 float orientation = 0.0f,
					 vec2 center = vec2(0.0f, 0.0f));

	 void drawString(const std::string &text, const vec2 &pos, const Color &color, float scale = 1.0f);

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
	void renderBatch(const Texture2D *batchTexture, SpriteInfo *first, int count);

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
	ShaderProgram *currentShader;
	ShaderProgram defaultShader;

	std::vector<SpriteInfo> spriteQueue; // Use dynamic array instead?
	bool inBeginEndPair;
};

#endif