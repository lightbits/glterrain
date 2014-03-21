#ifndef GL_CONTEXT_H
#define GL_CONTEXT_H
#include <common/timer.h>
#include <gl/glutils.h>
#include <app/context.h>

void crash(const char *error);

class GLContext : public Context
{
public:
	GLContext();
	~GLContext() { }
	
	bool create(const VideoMode &mode, const char *title, bool decorated, bool centered);
	void setActive();
	void close();
	void dispose();

	void pollEvents();
	void display();

	void setCursorEnabled(bool cursor);
	void setWindowTitle(const char *title);
	void setWindowPosition(int x, int y);
	void setWindowSize(int w, int h);
	void setVerticalSync(bool vsync);

	void getSize(int *width, int *height);
	int	 getWidth();
	int	 getHeight();
	void getMousePos(int *x, int *y);
	int  getMouseX();
	int  getMouseY();
	void setMousePos(int x, int y);

	bool isOpen();

	void sleep(double seconds);
	void sleepms(unsigned int milliseconds);
	double getElapsedTime();
};

GLContext *getActiveContext();

#endif