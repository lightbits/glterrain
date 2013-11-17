#include <app/glcontext.h>
#include <iostream>

static GLContext *activeContext = nullptr;
GLContext *getActiveContext() {
	return activeContext;
}

void crash(const char *error )
{
	std::cerr<<error<<std::endl;
	std::cin.get();
	exit(EXIT_FAILURE);
}

VideoMode::VideoMode() 
	: windowX(0), windowY(0), 
	windowWidth(640), windowHeight(480), 
	depthBits(0), stencilBits(0), 
	fsaaSamples(0), fullscreen(false)
{ }

VideoMode::VideoMode(int windowX_, int windowY_, 
		int windowWidth_, int windowHeight_, 
		int depthBits_, int stencilBits_, 
		int fsaaSamples_, bool fullscreen_) 
	: windowX(windowX_), windowY(windowY_), 
	windowWidth(windowWidth_), windowHeight(windowHeight_), 
	depthBits(depthBits_), stencilBits(stencilBits_), 
	fsaaSamples(fsaaSamples_), fullscreen(fullscreen_)
{ }

VideoMode::VideoMode(int windowWidth_, int windowHeight_, 
		int depthBits_, int stencilBits_, 
		int fsaaSamples_, bool fullscreen_) 
	: windowWidth(windowWidth_), windowHeight(windowHeight_), 
	depthBits(depthBits_), stencilBits(stencilBits_), 
	fsaaSamples(fsaaSamples_), fullscreen(fullscreen_)
{ }

GLContext::GLContext()
{

}

bool GLContext::create(const std::string &title, const VideoMode &vm, int major, int minor)
{
	if(glfwInit() != GL_TRUE)
		return false;

	glfwOpenWindowHint(GLFW_OPENGL_PROFILE, 0); // 0 = auto
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, major);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, minor);
	glfwOpenWindowHint(GLFW_FSAA_SAMPLES, vm.fsaaSamples);
	glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);

	int mode = vm.fullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW;
	if(glfwOpenWindow(vm.windowWidth, vm.windowHeight, 0, 0, 0, 0, vm.depthBits, vm.stencilBits, mode) != GL_TRUE)
		return false;

	//glfwSetWindowPos(vm.windowX, vm.windowY);
	glfwSetWindowTitle(title.c_str());
	glfwSwapInterval(1); // vsync (experimental)

	if(glload::LoadFunctions() == glload::LS_LOAD_FAILED)
		return false;

	std::cout<<"Debug context: "	<<(glfwGetWindowParam(GLFW_OPENGL_DEBUG_CONTEXT) ? "yes" : "no")<<std::endl;
	std::cout<<"HW accelerated: "	<<(glfwGetWindowParam(GLFW_ACCELERATED) ? "yes" : "no")<<std::endl;
	std::cout<<"Depth bits: "		<<glfwGetWindowParam(GLFW_DEPTH_BITS)<<std::endl;
	std::cout<<"Stencil bits: "		<<glfwGetWindowParam(GLFW_STENCIL_BITS)<<std::endl;
	std::cout<<"FSAA samples: "		<<glfwGetWindowParam(GLFW_FSAA_SAMPLES)<<std::endl;
	std::cout<<"Vendor: "			<<glGetString(GL_VENDOR)<<std::endl;
	std::cout<<"Renderer: "			<<glGetString(GL_RENDERER)<<std::endl;
	std::cout<<"GL ver.: "			<<glGetString(GL_VERSION)<<std::endl;
	std::cout<<"GLSL ver.: "		<<glGetString(GL_SHADING_LANGUAGE_VERSION)<<std::endl;

	return true;

	setActive();
}

void GLContext::setActive()
{
	activeContext = this;
}

void GLContext::close()
{
	glfwTerminate();
	activeContext = nullptr;
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

void GLContext::setWindowTitle(const std::string &title)
{
	glfwSetWindowTitle(title.c_str());
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
	if(vsync)
		glfwSwapInterval(1); // Experimental
	else
		glfwSwapInterval(0);
}

void getSize(int *width, int *height)
{
	glfwGetWindowSize(width, height);
}

void getMousePos(int *x, int *y)
{
	glfwGetMousePos(x, y);
}

void GLContext::sleep(double time)
{
	glfwSleep(time);
}

bool GLContext::isOpen()
{
	return glfwGetWindowParam(GLFW_OPENED) == GL_TRUE;
}
