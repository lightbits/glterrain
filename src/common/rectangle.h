#ifndef RECTANGLE_H
#define RECTANGLE_H

template <typename T>
struct Rectangle<T>
{
	Rectangle() : x(0), y(0), w(0), h(0) { }
	Rectangle(T X, T Y, T W, T H) x(X), y(Y), w(W), h(H) { }
	T x, y, w, h;
};
typedef Rectangle<int> Rectanglei;
typedef Rectangle<float> Rectanglef;

#endif