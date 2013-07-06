#ifndef SLGL_GRAPHICS_COLOR_H
#define SLGL_GRAPHICS_COLOR_H
#include <algorithm> // for std::min

namespace graphics
{

struct Color
{
	Color() : r(0), g(0), b(0), a(0) { }

	Color(float r_, float g_, float b_, float a_ = 1.0f) :
	r(std::min(r_, 1.0f)), 
	g(std::min(g_, 1.0f)), 
	b(std::min(b_, 1.0f)), 
	a(std::min(a_, 1.0f)) 
	{ }

	Color(float s) :
	r(std::min(s, 1.0f)), 
	g(std::min(s, 1.0f)), 
	b(std::min(s, 1.0f)), 
	a(1.0f)
	{ }

	Color(int r_, int g_, int b_, int a_ = 255) :
	r(std::min(r_ / 255.0f, 1.0f)), 
	g(std::min(g_ / 255.0f, 1.0f)), 
	b(std::min(b_ / 255.0f, 1.0f)), 
	a(std::min(a_ / 255.0f, 1.0f))
	{ }

	Color(int s) :
	r(std::min(s / 255.0f, 1.0f)), 
	g(std::min(s / 255.0f, 1.0f)), 
	b(std::min(s / 255.0f, 1.0f)), 
	a(1.0f)
	{ }

	bool operator==(const Color &rhs) const
	{
		return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a;
	}

	float r, g, b, a;
};

namespace Colors
{
	const Color Black = Color(0.0f, 0.0f, 0.0f, 1.0f);
	const Color White = Color(1.0f, 1.0f, 1.0f, 1.0f);
	const Color Transparent = Color(1.0f, 1.0f, 1.0f, 0.0f);
}

}

#endif