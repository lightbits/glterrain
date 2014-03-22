#ifndef CONTEXT_H
#define CONTEXT_H
#include <app/videomode.h>
#include <string>

class Context
{
//public:
//	enum Style
//	{
//		Border    = 0x10000000,
//		Centered  = 0x01000000
//	};

public:
	Context() { }
	virtual ~Context() { }
	
	virtual bool create(const VideoMode &mode, const char *title, bool decorated, bool centered) = 0;
	virtual void close() = 0;
	virtual void dispose() = 0;

	virtual std::string getDebugInfo() const = 0;

	virtual void pollEvents() = 0;
	virtual void display() = 0;

	virtual void setCursorEnabled(bool cursor) = 0;
	virtual void setWindowTitle(const char *title) = 0;
	virtual void setWindowPosition(int x, int y) = 0;
	virtual void setWindowSize(int w, int h) = 0;
	virtual void setActive() = 0;

	virtual void setVerticalSync(bool vsync) = 0;

	/* Retrieves the width and height of the window */
	virtual void getSize(int *width, int *height) const = 0;

	virtual int getWidth() const = 0;
	virtual int getHeight() const = 0;

	/* Returns the current mouse position. If the cursor is not hidden, the mouse
	position is the cursor position, relative to the upper left corner of the window and
	with the Y-axis down. */
	virtual void getMousePos(int *x, int *y) const = 0;
	virtual int  getMouseX() const = 0;
	virtual int  getMouseY() const = 0;
	virtual void setMousePos(int x, int y) = 0;

	virtual bool isOpen() const = 0;

	virtual void sleep(double seconds) = 0;
	virtual void sleepms(unsigned int milliseconds) = 0;

	/* Returns the time since the last call to create(), in seconds. */
	virtual double getElapsedTime() = 0;
private:
	Context(const Context &copy) { }
	Context &operator=(const Context &rhs) { }
};

#endif