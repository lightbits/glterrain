#ifndef RECTANGLE_H
#define RECTANGLE_H

struct Rectanglei
{
	Rectanglei() : x(0), y(0), w(0), h(0) { }
	Rectanglei(int X, int Y, int W, int H) : x(X), y(Y), w(W), h(H) { }
	int x, y, w, h;
};

struct Rectanglef
{
	Rectanglef() : x(0), y(0), w(0), h(0) { }
	Rectanglef(float X, float Y, float W, float H) : x(X), y(Y), w(W), h(H) { }
	float x, y, w, h;
};

#endif