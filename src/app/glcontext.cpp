#include <app/glcontext.h>
#include <gl/opengl.h>
#include <common/timer.h>
#include <sstream>
#include <string>

static GLContext *activeContext = nullptr;
GLContext *getActiveContext() {
	return activeContext;
}

GLContext::GLContext()
{

}

bool GLContext::create(const VideoMode &mode, const char *title, bool decorated, bool centered)
{
	if(glfwInit() != GL_TRUE)
		return false;

	glfwOpenWindowHint(GLFW_OPENGL_PROFILE, 0); // 0 = auto
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, mode.GlMajor);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, mode.GlMinor);
	glfwOpenWindowHint(GLFW_FSAA_SAMPLES, mode.FsaaSamples);
	glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);

	int fsFlag = mode.Fullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW;
	if(glfwOpenWindow(mode.Width, mode.Height, 0, 0, 0, 0, mode.DepthBits, mode.StencilBits, fsFlag) != GL_TRUE)
		return false;

	//glfwSetWindowPos(vm.windowX, vm.windowY);
	glfwSetWindowTitle(title);
	glfwSwapInterval(1); // vsync (experimental)

	if(glload::LoadFunctions() == glload::LS_LOAD_FAILED)
		return false;

	setActive();

	return true;
}

void GLContext::setActive()
{
	activeContext = this;
}

void GLContext::close()
{
	glfwCloseWindow();
	//glfwTerminate();
	//activeContext = nullptr;
}

void GLContext::dispose()
{
	glfwTerminate();
	activeContext = nullptr;
}

std::string GLContext::getDebugInfo() const
{
	std::stringstream ss;
	ss << "Debug context: "	 <<(glfwGetWindowParam(GLFW_OPENGL_DEBUG_CONTEXT) ? "yes" : "no") << std::endl;
	ss << "HW accelerated: " <<(glfwGetWindowParam(GLFW_ACCELERATED) ? "yes" : "no") << std::endl;
	ss << "Depth bits: "     <<glfwGetWindowParam(GLFW_DEPTH_BITS) << std::endl;
	ss << "Stencil bits: "   <<glfwGetWindowParam(GLFW_STENCIL_BITS) << std::endl;
	ss << "FSAA samples: "   <<glfwGetWindowParam(GLFW_FSAA_SAMPLES) << std::endl;
	ss << "Vendor: "         <<glGetString(GL_VENDOR) << std::endl;
	ss << "Renderer: "       <<glGetString(GL_RENDERER) << std::endl;
	ss << "GL ver.: "        <<glGetString(GL_VERSION) << std::endl;
	ss << "GLSL ver.: "	     <<glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	return ss.str();
}

void GLContext::pollEvents()
{
	glfwPollEvents();
}

void GLContext::display()
{
	glfwSwapBuffers();
}

void GLContext::setCursorEnabled(bool cursor)
{
	if(cursor)
		glfwEnable(GLFW_MOUSE_CURSOR);
	else
		glfwDisable(GLFW_MOUSE_CURSOR);
}

void GLContext::setWindowTitle(const char *title)
{
	glfwSetWindowTitle(title);
}

void GLContext::setWindowPosition(int x, int y)
{
	glfwSetWindowPos(x, y);
}

void GLContext::setWindowSize(int w, int h)
{
	glfwSetWindowSize(w, h);
}

void GLContext::setVerticalSync(bool vsync)
{
	if(vsync) glfwSwapInterval(1);
	else glfwSwapInterval(0);
}

void GLContext::getSize(int *width, int *height) const
{
	glfwGetWindowSize(width, height);
}

int GLContext::getWidth() const
{
	int w, h; getSize(&w, &h);
	return w;
}

int GLContext::getHeight() const
{
	int w, h; getSize(&w, &h);
	return h;
}

void GLContext::getMousePos(int *x, int *y) const
{
	glfwGetMousePos(x, y);
}

int GLContext::getMouseX() const
{ 
	int x, y;
	getMousePos(&x, &y);
	return x;
}

int GLContext::getMouseY() const
{
	int x, y;
	getMousePos(&x, &y);
	return y;
}		

void GLContext::setMousePos(int x, int y)
{
	glfwSetMousePos(x, y);
}

double GLContext::getElapsedTime()
{
	return glfwGetTime();
}

void GLContext::sleep(double seconds)
{
	glfwSleep(seconds);
}

void GLContext::sleepms(unsigned int milliseconds)
{
	glfwSleep(milliseconds / 1000.0);
}

bool GLContext::isOpen() const
{
	return glfwGetWindowParam(GLFW_OPENED) == GL_TRUE;
}
