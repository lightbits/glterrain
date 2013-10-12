#include <graphics/renderer.h>
using namespace graphics;

VideoMode::VideoMode() 
	: x(0), y(0), w(640), h(480), db(0), sb(0), fsaa(0), fs(0)
{ }

VideoMode::VideoMode(int windowX, int windowY, 
		int windowWidth, int windowHeight, 
		int depthBits, int stencilBits, 
		int fsaaSamples, bool fullscreen) 
	: x(windowX), y(windowY), w(windowWidth), h(windowHeight), 
	db(depthBits), sb(stencilBits), fsaa(fsaaSamples), fs(fullscreen)
{ }

VideoMode::VideoMode(int windowWidth, int windowHeight, 
		int depthBits, int stencilBits, 
		int fsaaSamples, bool fullscreen) 
	: x(0), y(0), w(windowWidth), h(windowHeight), 
	db(depthBits), sb(stencilBits), fsaa(fsaaSamples), fs(fullscreen)
{ }