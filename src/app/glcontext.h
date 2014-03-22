#ifndef GL_CONTEXT_H
#define GL_CONTEXT_H
#include <app/context.h>

class GLContext : public Context
{
public:
	GLContext();
	~GLContext() { }
	
	bool create(const VideoMode &mode, const char *title, bool decorated, bool centered);
	void setActive();
	void close();
	void dispose();

	std::string getDebugInfo() const;

	void pollEvents();
	void display();

	void setCursorEnabled(bool cursor);
	void setWindowTitle(const char *title);
	void setWindowPosition(int x, int y);
	void setWindowSize(int w, int h);
	void setVerticalSync(bool vsync);

	void getSize(int *width, int *height) const;
	int	 getWidth() const;
	int	 getHeight() const;
	void getMousePos(int *x, int *y) const;
	int  getMouseX() const;
	int  getMouseY() const;
	void setMousePos(int x, int y);

	bool isOpen() const;

	void sleep(double seconds);
	void sleepms(unsigned int milliseconds);
	double getElapsedTime();
};

GLContext *getActiveContext();

#endif