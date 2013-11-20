#include <graphics/spritebatch.h>
#include <app/context.h>

static const char *spritebatchVertexShaderSrc =
	"#version 150"
	"in vec4 vertColor;"
	"in vec2 vertTexCoord;"
	"out vec4 outColor;"
	"uniform float texBlend;"
	"uniform sampler2D tex;"
	"void main()"
	"{"
	"	vec4 multColor = mix(vec4(1, 1, 1, 1), vertColor, texBlend);"
	"	outColor = mix(vertColor, texture(tex, vertTexCoord), texBlend) * multColor;"
	"}";

static const char *spritebatchFragShaderSrc =
	"#version 150"
	"in vec3 position;"
	"in vec4 color;"
	"in vec2 texel;"
	"out vec4 vertColor;"
	"out vec2 vertTexCoord;"
	"uniform mat4 projection;"
	"uniform mat4 view;"
	"void main()"
	"{	"
	"	vertColor = color;"
	"	vertTexCoord = texel;"
	"	gl_Position = projection * view * vec4(position.x, position.y, position.z, 1.0f);"
	"}";

SpriteBatch::SpriteBatch()
{
	if(!defaultShader.loadFromSource(spritebatchVertexShaderSrc, spritebatchFragShaderSrc))
		throw std::exception("Failure loading default spritebatch shader");
	
	// Get window dimensions
	GLcontext *ar = getActiveContext();
	if(!ar)
		throw std::exception("There is no active context");
	int windowWidth, windowHeight;
	ar->getSize(&windowWidth, windowHeight);

	viewMatrix = mat4(1.0f);

	// Orthographic projection, with y-axis positive from top to bottom of window
	projectionMatrix = glm::ortho(0, windowWidth, windowHeight, 0);

	// Render states
	blendState = BlendStates::Default;
	sortMode = SpriteSortMode.None;

	// State variables
	currentFont = nullptr;
	currentShader = nullptr;
	inBeginEndPair = false;

	// Preallocate buffers
	vertexBuffer.create(GL_ARRAY_BUFFER, GL_STREAM_DRAW);
	vertexBuffer.bind();
	vertexBuffer.bufferData(SPRITE_COUNT * VERTICES_PER_SPRITE * VERTEX_SIZE);
	vertexBuffer.unbind();
	indexBuffer.create(GL_ELEMENT_ARRAY_BUFFER, GL_STREAM_DRAW);
	indexBuffer.bind();
	indexBuffer.bufferData(SPRITE_COUNT * INDICES_PER_SPRITE * INDEX_SIZE);
	indexBuffer.unbind();
}

SpriteBatch::~SpriteBatch()
{
	vertexBuffer.dispose();
	indexBuffer.dispose();
	defaultShader.dispose();
}

void SpriteBatch::begin(SpriteBlendMode blendMode, const mat4 &view)
{
	if(inBeginEndPair)
		throw std::exception("Begin cannot be called inside a begin-end pair");

	inBeginEndPair = true;
	viewMatrix = view;

	switch(blendMode)
	{
		case SpriteBlendMode.Additive: blendState = BlendStates::Additive; break;
		case SpriteBlendMode.AlphaBlend: blendState = BlendStates::AlphaBlend; break;
		case SpriteBlendMode.None: blendState = BlendStates::Default; break;
	}

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
	int first = 0;
	for(int i = 0; i < spriteQueue.size(); ++i)
	{
		const Texture *texture = spriteQueue[i].texture;

		// Issue draw call if there is a texture change and batch count > 0
		if(texture->getHandle() != currentTexture->getHandle())
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
	renderBatch(currentTexture, &spriteQueue[first], spriteQueue.size() - first);
	spriteQueue.clear();
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

	struct vertex
	{
		float x, y, z;
		float r, g, b, a;
		float u, v;
	};

	// Calculate vertex data on CPU before uploading to GPU
	Vertex *vertices = new vertex[count * 4];
	unsigned int *indices = new unsigned int[count * 6];

	// Iterate over each sprite in the batch, and calculate the vertices
	int vi = 0;
	int ii = 0;
	for(int i = 0; i < count; ++i)
	{
		SpriteInfo *sprite = (first + i);
		Rectanglef dest = sprite->destination;
		Rectanglei src = sprite->source;
		float zAxisRotation = sprite->zAxisRotation;
		Color color = sprite->color;
		float z = sprite->z;
		float x0 = dest.x;
		float x1 = dest.x + dest.w;
		float y0 = dest.y;
		float y1 = dest.y + dest.h;
		float r = color.r;
		float g = color.g;
		float b = color.b;
		float a = color.a;
		float u0 = src.x / float(textureWidth);
		float u1 = (src.x + src.w) / float(textureWidth);
		float v0 = src.y / float(textureHeight); // TODO: Upside down
		float v1 = (src.y + src.h) / float(textureHeight); // TODO: Upside down
		vertices[++vi] = { x0, y0, z, r, g, b, a, u0, v0 }; // Top-left
		vertices[++vi] = { x1, y0, z, r, g, b, a, u1, v0 }; // Top-right
		vertices[++vi] = { x1, y1, z, r, g, b, a, u1, v1 }; // Bottom-right
		vertices[++vi] = { x0, y1, z, r, g, b, a, u0, v1 }; // Bottom-left
		indices[++ii] = 0; indices[++ii] = 1; indices[++ii] = 2;
		indices[++ii] = 2; indices[++ii] = 3; indices[++ii] = 0;
	}

	vertexBuffer.bufferSubData(0, count * sizeof(Vertex), vertices);
	indexBuffer.bufferSubData(0, count * sizeof(unsigned int), indices);

	currentShader->setAttributefv("position", 3, GL_FLOAT, 9, 0);
	currentShader->setAttributefv("color", 4, GL_FLOAT, 9, 3);
	currentShader->setAttributefv("texel", 2, GL_FLOAT, 9, 7);
	currentShader->setUniform("tex", 0);
	currentShader->setUniform("view", viewMatrix);
	currentShader->setUniform("projection", projectionMatrix);
	// renderer->setTextureUnit(0);
	renderer->setBlendState(blendState);
	renderer-drawIndexedGeomtry(GL_TRIANGLES, count * 6, GL_UNSIGNED_INT);

	vertexBuffer.unbind();
	indexBuffer.unbind();
	texture-unbind();
	currentShader.end();

	delete[] vertices;
	delete[] indices;
}

void drawTexture(const Texture &texture, const Color &color, const Rectanglef &dest, const Rectanglef &srcnc, float depth, float orientation)
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
	spriteInfo.source = srcnc;
	spriteInfo.destination = dest;
	spriteInfo.texture = &texture;
	spriteQueue.push_back(spriteInfo);
}