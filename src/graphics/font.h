#ifndef FONT_H
#define FONT_H
#include <gl/texture.h>
#include <unordered_map>
#include <string>

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

	/* Calculate the width (TODO: and height) of the string, in pixel units */
	int measureString(const std::string &str) const;
	const Texture2D *getTexture() const;

	bool loadFromFile(const std::string &filename, const std::string &charSet = 
		" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~");
private:
	Texture2D *texture;
	std::unordered_map<char, Glyph> glyphs;
};

#endif