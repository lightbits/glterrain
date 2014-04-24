#ifndef GL_CONTEXT_H
#define GL_CONTEXT_H
#include <app/context.h>
#include <gl/opengl.h>

class GLContext : public Context
{
public:
	GLContext();
	
	bool create(const VideoMode &mode, const char *title, bool decorated, bool centered);
	void close();
	void dispose();

	std::string getDebugInfo() const;

	void pollEvents();
	void display();

	void screenshot(const char *filename, int x = 0, int y = 0, int w = 0, int h = 0);

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
	bool isKeyPressed(int scancode) const;
	bool isKeyPressed(char letter) const;
	bool isMousePressed(int button) const;

	void sleep(double seconds);
	void sleepms(unsigned int milliseconds);
	double getElapsedTime();
private:
	SDL_Window *window;
	SDL_GLContext context;
};

GLContext *getActiveContext();

#endif