#include <graphics/spritebatch.h>
#include <sstream>
#include <iostream>
using namespace graphics;

SpriteBatch::SpriteBatch() : 
vbo(),
ibo(),
currentFont(nullptr), 
currentTexture(nullptr), 
currentProgram(nullptr),
elementIndex(0),
vertexIndex(0),
positionAttribLocation(0),
colorAttribLocation(0),
texCoordAttribLocation(0),
projectionUniform(0),
texBlendUniform(0),
textureSamplerUniform(0),
viewUniform(0),
projectionMatrix(1.0f),
viewMatrix(1.0f)
{
	
}

void SpriteBatch::dispose()
{
	vbo.dispose();
	ibo.dispose();
}

void SpriteBatch::create(const Program &program)
{
	currentProgram = &program;
	positionAttribLocation	= program.getAttribLocation("position");
	colorAttribLocation		= program.getAttribLocation("color");
	texCoordAttribLocation	= program.getAttribLocation("texel");

	projectionUniform		= program.getUniformLocation("projection");
	viewUniform				= program.getUniformLocation("view");
	texBlendUniform			= program.getUniformLocation("texBlend");
	textureSamplerUniform	= program.getUniformLocation("tex");

	int wWidth, wHeight;
	glfwGetWindowSize(&wWidth, &wHeight);
	projectionMatrix = glm::ortho(0.0f, (float)wWidth, (float)wHeight, 0.0f, 1.0f, 10.0f);

	// Preallocate memory for vertex buffer
	vbo.create(GL_ARRAY_BUFFER, GL_STREAM_DRAW);
	vbo.bind();
	vbo.bufferData(SPRITE_BATCH_VBO_SIZE, NULL);

	// Preallocate memory for the index buffer
	ibo.create(GL_ELEMENT_ARRAY_BUFFER, GL_STREAM_DRAW);
	ibo.bind();
	ibo.bufferData(SPRITE_BATCH_IBO_SIZE, NULL);

	// Remember to unbind! Having a bound element array buffer can be problematic to debug
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void SpriteBatch::begin()
{
	if(currentProgram == nullptr)
	{
		std::cerr<<"Error: No program is set for the sprite batch"<<std::endl;
		return;
	}

	vbo.bind();
	ibo.bind();

	currentProgram->uniform(projectionUniform, projectionMatrix);
	currentProgram->uniform(viewUniform, viewMatrix);

	// The vertex format will be shared by all incoming data, so we specify it here
	glEnableVertexAttribArray(positionAttribLocation);
	glVertexAttribPointer(positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(colorAttribLocation);
	glVertexAttribPointer(colorAttribLocation, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(texCoordAttribLocation);
	glVertexAttribPointer(texCoordAttribLocation, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (void*)(7 * sizeof(GLfloat)));

	elementIndex = 0;
	vertexIndex = 0;
}

void SpriteBatch::flush()
{
	// Switch texture (if any)
	if(currentTexture != nullptr)
	{
		currentTexture->bind();
		currentProgram->uniform(texBlendUniform, 1.0f);
		currentProgram->uniform(textureSamplerUniform, 0); // Using texture unit 0
	}
	else
	{
		currentProgram->uniform(texBlendUniform, 0.0f);
	}

	glDrawElements(GL_TRIANGLES, elementIndex, GL_UNSIGNED_SHORT, 0);

	elementIndex = 0;
	vertexIndex = 0;
}

void SpriteBatch::end()
{
	flush();

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisableVertexAttribArray(positionAttribLocation);
	glDisableVertexAttribArray(colorAttribLocation);
	glDisableVertexAttribArray(texCoordAttribLocation);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void SpriteBatch::setTexture(const Texture *texture)
{
	if(currentTexture != texture)
		flush();
	currentTexture = texture;
}

void SpriteBatch::draw(const Texture *texture, const Color &color, 
		const vec3 &v0, const vec3 &v1, const vec3 &v2, const vec3 &v3, 
		float ul, float ur, float vb, float vt)
{
	setTexture(texture);

	// Flip vertical texture-coordinate to compensate for the flipped viewport
	std::swap(vb, vt);

	const GLfloat vertices[36] = {
		// Position				Color									Texture coordinates
		v0.x, v0.y, v0.z,		color.r, color.g, color.b, color.a,		ul, vb, // Bottom-left
		v1.x, v1.y, v1.z,		color.r, color.g, color.b, color.a,		ur, vb, // Bottom-right
		v2.x, v2.y, v2.z,		color.r, color.g, color.b, color.a,		ur, vt, // Top-right
		v3.x, v3.y, v3.z,		color.r, color.g, color.b, color.a,		ul, vt	// Top-left
	};

	// Note that we specify the indices opposite of the winding order, as
	// we perform a viewport flip, such that the winding order is reversed in raster coordinates.
	GLushort i = vertexIndex;
	const GLushort CWindices[6] = {i, i + 1, i + 2, i + 2, i + 3, i};
	const GLushort CCWindices[6] = {i, i + 3, i + 2, i + 2, i + 1, i};

	vbo.bufferSubData(vertexIndex * 9 * sizeof(GLfloat), sizeof(vertices), vertices);
	ibo.bufferSubData(elementIndex * sizeof(GLushort), sizeof(CWindices), (SPRITE_BATCH_WINDING_ORDER == GL_CW ? CWindices : CCWindices));

	elementIndex += 6;
	vertexIndex += 4;

	/*
	assert(sizeof(vertices) == 4 * SPRITE_BATCH_VERTEX_SIZE);
	assert(sizeof(indices) == 6 * sizeof(GLushort));
	*/
}

void SpriteBatch::drawTexture(const Texture &texture, const Color &color, 
float dstX, float dstY, float dstW, float dstH,
float ul, float ur, float vb, float vt)
{
	// Creating all these vecs might be slow?
	draw(&texture, color, 
		vec3(dstX, dstY, -1.0f),
		vec3(dstX + dstW, dstY, -1.0f),
		vec3(dstX + dstW, dstY + dstH, -1.0f),
		vec3(dstX, dstY + dstH, -1.0f),
		ul, ur, vb, vt);
}

void SpriteBatch::drawTexture(const Texture &texture, const Color &color, 
float dstX, float dstY, float dstW, float dstH, 
int srcX, int srcY, int srcW, int srcH)
{
	float ul = srcX / (float)texture.getWidth();
	float ur = (srcX + srcW) / (float)texture.getWidth();
	float vb = (texture.getHeight() - (srcY + srcH)) / (float)texture.getHeight();
	float vt = (texture.getHeight() - srcY) / (float)texture.getHeight();
	drawTexture(texture, color, dstX, dstY, dstW, dstH, ul, ur, vb, vt);
}

void SpriteBatch::drawTexture(const Texture &texture, const Color &color, 
float dstX, float dstY, float dstW, float dstH)
{
	drawTexture(texture, color, dstX, dstY, dstW, dstH, 0.0f, 1.0f, 0.0f, 1.0f);
}

void SpriteBatch::drawQuad(const Color &color, float x, float y, float w, float h)
{
	draw(nullptr, color, 
		vec3(x, y, -1.0f),
		vec3(x + w, y, -1.0f),
		vec3(x + w, y + h, -1.0f),
		vec3(x, y + h, -1.0f),
		0, 0, 0, 0);
}

void SpriteBatch::drawString(const std::string &text, float x, float y, const Color &color)
{
	if(currentFont == nullptr)
	{
		std::cerr<<"Error: No font is set."<<std::endl;
		return;
	}

	std::stringstream stream(text);
	std::vector<std::string> lines;
	std::string line;
	while(stream.good())
	{
		std::getline(stream, line);
		lines.push_back(line);
	}

	float posY = y;
	float lineHeight = (float)currentFont->getGlyph('a').height;
	const Texture *texture = currentFont->getTexture();
	float txtwidth = (float)texture->getWidth();
	float txtheight = (float)texture->getHeight();
	for(int i = 0; i < lines.size(); ++i)
	{
		float posX = x;
		for(int j = 0; j < lines[i].size(); ++j)
		{
			Glyph glyph = currentFont->getGlyph(lines[i][j]);

			drawTexture(*texture, color, 
				posX, posY, glyph.width, glyph.height,
				glyph.uLeft, glyph.uRight, glyph.vBottom, glyph.vTop);

			posX += glyph.width;
		}
		posY += lineHeight;
	}
}

void SpriteBatch::setFont(const Font &font)
{
	currentFont = &font;
}