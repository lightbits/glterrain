#ifndef COLOR_H
#define COLOR_H
#include <algorithm> // for std::min

struct Color
{
	Color() : 
		r(0), g(0), b(0), a(0) { }
	Color(float R, float G, float B, float A = 1.0f) : 
		r(R), g(G), b(B), a(A) { }
	Color(float s) : 
		r(s), g(s), b(s), a(1.0f) { }
	Color(int R, int G, int B, int A = 255) :
		r(R / 255.0f), g(G / 255.0f), b(B / 255.0f), a(A / 255.0f) { }
	Color(int s) :
		r(s / 255.0f), g(s / 255.0f), b(s / 255.0f), a(1.0f) { }

	bool operator==(const Color &rhs) const
	{
		return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a;
	}

	bool operator!=(const Color &rhs) const
	{
		return r != rhs.r || g != rhs.g || b != rhs.b || a != rhs.a;
	}

	float r, g, b, a;
};

namespace Colors
{
	const Color Black = Color(0.0f, 0.0f, 0.0f, 1.0f);
	const Color White = Color(1.0f, 1.0f, 1.0f, 1.0f);
	const Color Transparent = Color(1.0f, 1.0f, 1.0f, 0.0f);
}

#endif