#ifndef COLOR_H
#define COLOR_H
#include <algorithm> // for std::min

struct Color
{
	Color();
	Color(float R, float G, float B, float A = 1.0f);
	Color(float s);
	Color(int R, int G, int B, int A = 255);
	Color(int s);
	Color(unsigned int hex);

	bool operator==(const Color &rhs) const;
	bool operator!=(const Color &rhs) const;

	float r, g, b, a;
};

namespace Colors
{
	const Color Black = Color(0.0f, 0.0f, 0.0f, 1.0f);
	const Color White = Color(1.0f, 1.0f, 1.0f, 1.0f);
	const Color Transparent = Color(1.0f, 1.0f, 1.0f, 0.0f);
}

#endif