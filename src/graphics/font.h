#ifndef SLGL_GRAPHICS_FONT_H
#define SLGL_GRAPHICS_FONT_H
#include <graphics/texture.h>
#include <map>
#include <string>

namespace graphics
{

struct Glyph
{
	Glyph() : symbol('0'), uLeft(0), uRight(0), vBottom(0), vTop(0), width(0), height(0) { }

	char symbol;
	int width, height;
	float uLeft, uRight, vBottom, vTop; // OpenGL texture coordinates

};

class Font
{
friend class Graphics;
public:
	Font();
	~Font();

	void dispose();

	/*
	Returns the glyph corresponding to the given letter.
	Note that the area defined by the glyph is relative to the bottom-left corner
	of the texture.
	*/
	Glyph getGlyph(char c) const;
	int measureString(const std::string &str) const;
	const Texture *getTexture() const;

	bool loadFromFile(const std::string &filename, const std::string &charSet = 
		" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~");
private:
	Texture *texture;
	std::map<char, Glyph> glyphs;
};

}

#endif