#ifndef GL_CONTEXT_H
#define GL_CONTEXT_H
#include <string>
#include <gl/glutils.h>

void crash(const char *error);

struct VideoMode
{
	VideoMode();
	VideoMode(int windowX, int windowY, 
		int windowWidth, int windowHeight, 
		int depthBits, int stencilBits, 
		int fsaaSamples, bool fullscreen);

	VideoMode(int windowWidth, int windowHeight, 
		int depthBits, int stencilBits, 
		int fsaaSamples, bool fullscreen);

	int windowX, windowY;
	int windowWidth, windowHeight; 
	int depthBits, stencilBits; 
	int fsaaSamples;
	bool fullscreen;
};

class GLContext
{
public:
	GLContext();
	~GLContext() { }

	/* Requests a new OpenGL context and loads GL functions */
	bool create(const std::string &title, const VideoMode &vm, int major = 3, int minor = 1);
	void setActive();
	void close();

	void pollEvents();
	void display();

	void setCursorEnabled(bool cursor);
	void setWindowTitle(const std::string &title);
	void setWindowPosition(int x, int y);
	void setWindowSize(int w, int h);

	void setFullscreen(bool fullscreen);
	void setVerticalSync(bool vsync);

	/* Sets the context-thread to sleep for a given number of seconds */
	void sleep(double time);

	bool isOpen();
private:
	/* Disable copying */
	GLContext(const GLContext &copy);
	GLContext &operator=(const GLContext &rhs);
};

#endif