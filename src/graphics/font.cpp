#include <graphics/font.h>
#include <graphics/color.h>
#include <iostream>
using namespace graphics;
typedef unsigned char uint8;
typedef unsigned int uint32;

Font::Font() : glyphs(), texture(nullptr)
{

}

Font::~Font()
{
	if(texture != nullptr)
		delete texture;
}

void Font::dispose()
{
	if(texture != nullptr)
		texture->dispose();
}

Glyph Font::getGlyph(char c) const
{
	Glyph ret;
	if(glyphs.find(c) != glyphs.end())
		ret = glyphs.find(c)->second;
	return ret;
}

int Font::measureString(const std::string &str) const
{
	int width = 0;
	for(unsigned int i = 0; i < str.size(); ++i)
	{
		width += getGlyph(str[i]).width;
	}
	return width;
}

const Texture *Font::getTexture() const
{
	return texture;
}

/*
Returns the color of the pixel at [x, y] units from origin.
Assuming data is stored row-wise, RGBA.
*/
Color getPixel(uint32 x, uint32 y, uint8 *pixels, uint32 tWidth, uint32 tHeight)
{
	if(x >= tWidth || y >= tHeight)
		return Color();

	uint32 offset = 4 * (y * tWidth + x);

	return Color(
		pixels[offset + 0], 
		pixels[offset + 1],
		pixels[offset + 2],
		pixels[offset + 3]
	);
}

/*
Loads the texture of the given filename, containing the glyphs.
Pixels are obtained by binding the texture object and calling glGetTexImage
for the array of pixels
*/
bool Font::loadFromFile(const std::string &filename, const std::string &charSet)
{
	// Dispose previous texture, if any
	dispose();

	// Deallocate allocated memory, if any (not done in dispose() method)
	if(texture != nullptr)
		delete texture;

	texture = new Texture();
	if(!texture->loadFromFile(filename))
		return false;

	// We want our text to appear crisp and clear
	texture->setSmooth(false);

	// Get (actual) texture dimensions
	glBindTexture(GL_TEXTURE_2D, texture->getHandle());
	GLint textureWidth = 0;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &textureWidth);
	GLint textureHeight = 0;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &textureHeight);

	if(textureWidth == 0 || textureHeight == 0)
		return false;

	// Get raw image data so we can investigate the pixels
	unsigned char *pixels = new unsigned char[textureWidth * textureHeight * 4];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	// OpenGL defines the origin of the texture to be in the bottom-left corner.
	// So we search from the top of the texture and downwards.
	unsigned int index = 0;
	for(int y = textureHeight - 1; y >= 0; --y)
	{
		int skipHeight = 0;
		for(int x = 0; x < textureWidth; ++x)
		{
			Color color = getPixel(x, y, pixels, textureWidth, textureHeight);

			// Glyph box detected
			if(color == Color(0, 0, 0, 255) && index != charSet.size())
			{
				// Determine width of the glyph
				int w = 1;
				while(w < textureWidth - x && getPixel(x + w, y, pixels, textureWidth, textureHeight) == Color(0, 0, 0, 255))
					++w;

				// Determine height of the glyph by traversing pixels downwards
				int h = 1;
				// The y-coordinate is how high up on the texture we are
				// h can not exceed this value, as we will then go below the texture
				while(h <= y && getPixel(x, y - h, pixels, textureWidth, textureHeight) == Color(0, 0, 0, 255))
					++h;

				Glyph glyph;
				glyph.symbol = charSet[index++];
				glyph.width = w - 2;
				glyph.height = h - 2;
				glyph.uLeft = (x + 1) / (float)textureWidth;
				glyph.uRight = (x + w - 1) / (float)textureWidth;
				glyph.vBottom = (y - h + 2) / (float)textureHeight;
				glyph.vTop = y / (float)textureHeight;

				glyphs[glyph.symbol] = glyph;

				x += w;
				if(h > skipHeight)
					skipHeight = h;
			}
		}

		// Skip pixels (downwards) if we found any in glyphs the last horizontal-scan
		y -= skipHeight;
	}

	// Clean-up
	glBindTexture(GL_TEXTURE_2D, 0);
	delete[] pixels;

	return true;
}