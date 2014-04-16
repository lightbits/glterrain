#include <graphics/spritebatch.h>
#include <app/glcontext.h>
#include <graphics/renderer.h>
#include <sstream> // For drawstring

static const char *spritebatchFragShaderSrc =
	"#version 150\n"
	"in vec4 vertColor;\n"
	"in vec2 vertTexCoord;\n"
	"out vec4 outColor;\n"
	"uniform sampler2D tex;\n"
	"void main()\n"
	"{\n"
	"	outColor = texture(tex, vertTexCoord) * vertColor;\n"
	"}\n";

static const char *spritebatchVertexShaderSrc =
	"#version 150\n"
	"in vec3 position;\n"
	"in vec4 color;\n"
	"in vec2 texel;\n"
	"out vec4 vertColor;\n"
	"out vec2 vertTexCoord;\n"
	"uniform mat4 projection;\n"
	"uniform mat4 view;\n"
	"void main()\n"
	"{\n"
	"	vertColor = color;\n"
	"	vertTexCoord = texel;\n"
	"	gl_Position = projection * view * vec4(position.xyz, 1.0f);\n"
	"}\n";

SpriteBatch::SpriteBatch()
{
	currentFont = nullptr;
	currentShader = nullptr;
	inBeginEndPair = false;
}

SpriteBatch::~SpriteBatch()
{
	
}

void SpriteBatch::create()
{
	if(!defaultShader.loadFromSource(spritebatchVertexShaderSrc, spritebatchFragShaderSrc))
		throw std::exception("Failure loading default spritebatch shader");

	if(!defaultShader.linkAndCheckStatus())
		throw std::exception("Failure linking default spritebatch shader");
	
	// Get window dimensions
	GLContext *ar = getActiveContext();
	if(!ar)
		throw std::exception("There is no active context");
	int windowWidth, windowHeight;
	ar->getSize(&windowWidth, &windowHeight);

	viewMatrix = mat4(1.0f);

	// Orthographic projection, with y-axis positive from top to bottom of window
	projectionMatrix = glm::ortho(0.0f, float(windowWidth), float(windowHeight), 0.0f, 0.0f, 1.0f);

	// Render states
	blendState = BlendStates::Default;
	sortMode = SpriteSortMode::None;

	// State variables
	currentFont = nullptr;
	currentShader = nullptr;
	inBeginEndPair = false;

	// Preallocate buffers
	vertexBuffer.create(GL_ARRAY_BUFFER, GL_STREAM_DRAW);
	vertexBuffer.bind();
	vertexBuffer.bufferData(SPRITE_COUNT * VERTICES_PER_SPRITE * VERTEX_SIZE, NULL);
	vertexBuffer.unbind();
	indexBuffer.create(GL_ELEMENT_ARRAY_BUFFER, GL_STREAM_DRAW);
	indexBuffer.bind();
	indexBuffer.bufferData(SPRITE_COUNT * INDICES_PER_SPRITE * INDEX_SIZE, NULL);
	indexBuffer.unbind();
}

void SpriteBatch::dispose()
{
	vertexBuffer.dispose();
	indexBuffer.dispose();
	defaultShader.dispose();
}

void SpriteBatch::setFont(const Font &font)
{
	currentFont = &font;
}

void SpriteBatch::begin(BlendState blendMode, const mat4 &view)
{
	if(inBeginEndPair)
		throw std::exception("Begin cannot be called inside a begin-end pair");

	inBeginEndPair = true;
	viewMatrix = view;
	blendState = blendMode;

	currentShader = &defaultShader;
	spriteQueue.clear();
}

void SpriteBatch::end()
{
	if(!inBeginEndPair)
		throw std::exception("End cannot be called outside a begin-end pair");

	inBeginEndPair = false;

	// sortSprites();

	const Texture *currentTexture = nullptr;
	unsigned int first = 0;
	for(unsigned int i = 0; i < spriteQueue.size(); ++i)
	{
		const Texture *texture = spriteQueue[i].texture;

		// Issue draw call if there is a texture change and batch count > 0
		/*if(currentTexture == nullptr || (texture->getHandle() != currentTexture->getHandle()))
		{*/
		if(texture != currentTexture)
		{
			if(first != i)
			{
				renderBatch(currentTexture, &spriteQueue[first], i - first);
				first = i;
			}
			currentTexture = texture;
		}
	}

	// Draw remainder of batch
	if(spriteQueue.size() > 0)
	{
		renderBatch(currentTexture, &spriteQueue[first], spriteQueue.size() - first);
		spriteQueue.clear();
	}
}

void SpriteBatch::renderBatch(const Texture *texture, SpriteInfo *first, int count)
{
	Renderer *renderer = getActiveRenderer();
	if(!renderer)
		throw std::exception("No active renderer");

	currentShader->begin();

	// Prepare buffers
	vertexBuffer.bind();
	indexBuffer.bind();

	// Enable texture and get texture dimensions for calculating UV-coordinates
	texture->bind();
	int textureWidth, textureHeight;
	texture->getInternalSize(&textureWidth, &textureHeight);

	struct Vertex
	{
		Vertex() : x(0), y(0), z(0), r(0), g(0), b(0), a(0), u(0), v(0) { }
		Vertex(float X, float Y, float Z, float R, float G, float B, float A, float U, float V) : 
			x(X), y(Y), z(Z), r(R), g(G), b(B), a(A), u(U), v(V) { }
		float x, y, z;
		float r, g, b, a;
		float u, v;
	};

	// Calculate vertex data on CPU before uploading to GPU
	Vertex *vertices = new Vertex[count * 4];
	unsigned int *indices = new unsigned int[count * 6];

	// Iterate over each sprite in the batch, and calculate the vertices
	int vi = 0;
	int ii = 0;
	for(int i = 0; i < count; ++i)
	{
		SpriteInfo *sprite = (first + i);
		Rectanglef dest = sprite->destination;
		float zAxisRotation = sprite->zAxisRotation;
		Color color = sprite->color;
		vec2 center = sprite->center;
		float z = sprite->z;
		float r = color.r;
		float g = color.g;
		float b = color.b;
		float a = color.a;
		float ul = sprite->uLeft;
		float ur = sprite->uRight;
		float vb = sprite->vBottom;
		float vt = sprite->vTop;

		indices[ii++] = vi; indices[ii++] = vi + 1; indices[ii++] = vi + 2;
		indices[ii++] = vi + 2; indices[ii++] = vi + 3; indices[ii++] = vi;

		// Rotate the vertices if the rotation is nonzero
		if(abs(zAxisRotation) > 1.0e-3)
		{
			vec2 vert0((-center.x) * dest.w, (-center.y) * dest.h);
			vec2 vert1((1.0f - center.x) * dest.w, (-center.y) * dest.h);
			vec2 vert2((1.0f - center.x) * dest.w, (1.0f - center.y) * dest.h);
			vec2 vert3((-center.x) * dest.w, (1.0f - center.y) * dest.h);

			//vec2 center(0.0f, 0.0f);

			float cost = cos(zAxisRotation);
			float sint = -sin(zAxisRotation);
			mat2 rot(cost, sint, -sint, cost);
			vert0 = rot * vert0; vert1 = rot * vert1;
			vert2 = rot * vert2; vert3 = rot * vert3;

			vertices[vi++] = Vertex(vert0.x + dest.x, vert0.y + dest.y, z, r, g, b, a, ul, vt); // Top-left
			vertices[vi++] = Vertex(vert1.x + dest.x, vert1.y + dest.y, z, r, g, b, a, ur, vt); // Top-right
			vertices[vi++] = Vertex(vert2.x + dest.x, vert2.y + dest.y, z, r, g, b, a, ur, vb); // Bottom-right
			vertices[vi++] = Vertex(vert3.x + dest.x, vert3.y + dest.y, z, r, g, b, a, ul, vb); // Bottom-left
		}
		else
		{
			float x0 = dest.x - center.x * dest.w;
			float x1 = dest.x + dest.w - center.x * dest.w;
			float y0 = dest.y - center.y * dest.h;
			float y1 = dest.y + dest.h - center.y * dest.h;

			vertices[vi++] = Vertex(x0, y0, z, r, g, b, a, ul, vt); // Top-left
			vertices[vi++] = Vertex(x1, y0, z, r, g, b, a, ur, vt); // Top-right
			vertices[vi++] = Vertex(x1, y1, z, r, g, b, a, ur, vb); // Bottom-right
			vertices[vi++] = Vertex(x0, y1, z, r, g, b, a, ul, vb); // Bottom-left
		}		
	}

	// Upload vertex and index data to GPU
	vertexBuffer.bufferSubData(0, count * 4 * 9 * sizeof(float), vertices);
	indexBuffer.bufferSubData(0, count * 6 * sizeof(unsigned int), indices);

	// Set attribute format
	currentShader->setAttributefv("position", 3, 9, 0);
	currentShader->setAttributefv("color", 4, 9, 3);
	currentShader->setAttributefv("texel", 2, 9, 7);

	// Set texture source and transformation matrices
	currentShader->setUniform("tex", 0);
	currentShader->setUniform("view", viewMatrix);
	currentShader->setUniform("projection", projectionMatrix);

	// Set render states and issue draw call
	// renderer->setTextureUnit(0);
	renderer->setBlendState(blendState);
	renderer->setCullState(CullStates::CullNone);
	renderer->setRasterizerState(RasterizerStates::Default);
	renderer->setDepthTestState(DepthTestStates::Always);
	renderer->drawIndexedVertexBuffer(GL_TRIANGLES, count * 6, GL_UNSIGNED_INT);

	vertexBuffer.unbind();
	indexBuffer.unbind();
	texture->unbind();
	currentShader->end();

	delete[] vertices;
	delete[] indices;
}

void SpriteBatch::drawTexture(const Texture &texture,
					 const Color &color,
					 const Rectanglef &dest,
					 float uLeft, float uRight,
					 float vBottom, float vTop,
					 float depth,
					 float orientation,
					 vec2 center)
{
	// Draw the entire batch if max size is reached
	if(spriteQueue.size() >= SPRITE_COUNT)
	{
		end();
		begin(blendState, viewMatrix);
	}

	SpriteInfo spriteInfo;
	spriteInfo.z = depth;
	spriteInfo.color = color;
	spriteInfo.zAxisRotation = orientation;
	spriteInfo.uLeft = uLeft;
	spriteInfo.uRight = uRight;
	spriteInfo.vBottom = vBottom;
	spriteInfo.vTop = vTop;
	spriteInfo.destination = dest;
	spriteInfo.texture = &texture;
	spriteInfo.center = center;
	spriteQueue.push_back(spriteInfo);
}

void SpriteBatch::drawTexture(const Texture &texture, 
					const Color &color, 
					const Rectanglef &dest, 
					const Rectanglei &src, 
					float depth, float orientation,
					vec2 center)
{
	float w = float(texture.getWidth());
	float h = float(texture.getHeight());
	float uLeft = src.x / w;
	float uRight = (src.x + src.w) / w;
	float vBottom = 1.0f - (src.h + src.y) / h;
	float vTop = 1.0f - src.y / h;
	drawTexture(texture, color, dest, uLeft, uRight, vBottom, vTop, depth, orientation, center);
}

void SpriteBatch::drawTexture(const Texture &texture, 
					const Color &color, 
					const Rectanglef &dest,
					float depth, float orientation,
					vec2 center)
{
	drawTexture(texture, color, dest, 0.0f, 1.0, 0.0f, 1.0f, depth, orientation, center);
}

void SpriteBatch::drawTexture(const Texture &texture, 
					const Color &color, 
					const vec2 &pos,
					const Rectanglei &src,
					float depth, float orientation,
					vec2 center)
{
	Rectanglef dest(pos.x, pos.y, float(texture.getWidth()), float(texture.getHeight()));
	drawTexture(texture, color, dest, src, depth, orientation, center);
}

void SpriteBatch::drawTexture(const Texture &texture, 
					const Color &color, 
					const vec2 &pos,
					float depth, float orientation,
					vec2 center)
{
	Rectanglef dest(pos.x, pos.y, float(texture.getWidth()), float(texture.getHeight()));
	drawTexture(texture, color, dest, 0.0f, 1.0, 0.0f, 1.0f, depth, orientation, center);
}

void SpriteBatch::drawString(const std::string &text, const vec2 &pos, const Color &color)
{
	if(currentFont == nullptr)
		throw std::exception("No font is set");

	std::stringstream stream(text);
	std::vector<std::string> lines;
	std::string line;
	while(stream.good())
	{
		std::getline(stream, line);
		lines.push_back(line);
	}

	float posY = pos.y;
	float lineHeight = (float)currentFont->getGlyph('a').height;
	const Texture *texture = currentFont->getTexture();
	float txtwidth = (float)texture->getWidth();
	float txtheight = (float)texture->getHeight();
	for(unsigned int i = 0; i < lines.size(); ++i)
	{
		float posX = pos.x;
		for(unsigned int j = 0; j < lines[i].size(); ++j)
		{
			Glyph glyph = currentFont->getGlyph(lines[i][j]);
			drawTexture(
				*texture, color, 
				Rectanglef(posX, posY, float(glyph.width), float(glyph.height)),
				glyph.uLeft, glyph.uRight, glyph.vBottom, glyph.vTop);
			posX += glyph.width;
		}
		posY += lineHeight;
	}
}