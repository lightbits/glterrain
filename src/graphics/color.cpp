#include <graphics/color.h>

Color::Color() : 
	r(0), g(0), b(0), a(0) 
{ }

Color::Color(float R, float G, float B, float A) : 
	r(R), g(G), b(B), a(A) 
{ }

Color::Color(float s) : 
	r(s), g(s), b(s), a(1.0f) 
{ }

Color::Color(int R, int G, int B, int A) :
	r(R / 255.0f), g(G / 255.0f), b(B / 255.0f), a(A / 255.0f) 
{ }

Color::Color(unsigned char R, unsigned char G, unsigned char B, unsigned char A) :
	r(R / 255.0f), g(G / 255.0f), b(B / 255.0f), a(A / 255.0f) 
{ }

Color::Color(unsigned char s) :
	r(s / 255.0f), g(s / 255.0f), b(s / 255.0f), a(1.0f) 
{ }

Color Color::fromHex(unsigned int hex) 
{
	return Color(
		((hex>>24) & 0xFF) / 255.0f, 
		((hex>>16) & 0xFF) / 255.0f, 
		((hex>> 8) & 0xFF) / 255.0f, 
		(hex & 0xFF) / 255.0f);
}

bool Color::operator==(const Color &rhs) const
{
	return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a;
}

bool Color::operator!=(const Color &rhs) const
{
	return r != rhs.r || g != rhs.g || b != rhs.b || a != rhs.a;
}