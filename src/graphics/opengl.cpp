#include <graphics/opengl.h>
#include <iostream>
using namespace graphics;

const char *gl::getErrorMessage(GLenum code)
{
	switch(code)
	{
	case 0: return "NO_ERROR";
	case 0x0500: return "INVALID_ENUM";
	case 0x0501: return "INVALID_VALUE";
	case 0x0502: return "INVALID_OPERATION";
	case 0x0503: return "STACK_OVERFLOW";
	case 0x0504: return "STACK_UNDERFLOW";
	case 0x0505: return "OUT_OF_MEMORY";
	case 0x0506: return "INVALID_FRAMEBUFFER_OPERATION";
	default: return "UNKNOWN";
	}
}

bool gl::createContext(const char *title, int x, int y, int width, int height, 
int depthbits, int stencilbits, int fsaa, bool fullscreen)
{
	if(glfwInit() != GL_TRUE)
		return false;

	glfwOpenWindowHint(GLFW_OPENGL_PROFILE, 0); // 0 lets the system choose the profile
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, OPENGL_VERSION_MAJOR);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, OPENGL_VERSION_MINOR);
	glfwOpenWindowHint(GLFW_FSAA_SAMPLES, fsaa);
	glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);

	if(glfwOpenWindow(
		width, height,
		0, 0, 0, 0, // Default color bits
		depthbits,
		stencilbits,
		(fullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW)) != GL_TRUE)
		return false;

	glfwSetWindowPos(x, y);
	glfwSetWindowTitle(title);

	if(glload::LoadFunctions() == glload::LS_LOAD_FAILED)
		return false;

	std::cout<<"Hello World!"<<std::endl<<"Running OpenGL ver. ";
	std::cout<<glfwGetWindowParam(GLFW_OPENGL_VERSION_MAJOR)<<".";
	std::cout<<glfwGetWindowParam(GLFW_OPENGL_VERSION_MINOR)<<std::endl;
	std::cout<<"Debug context: "<<(glfwGetWindowParam(GLFW_OPENGL_DEBUG_CONTEXT) ? "true" : "false")<<std::endl;
	std::cout<<"HW accelerated: "<<(glfwGetWindowParam(GLFW_ACCELERATED) ? "true" : "false")<<std::endl;
	std::cout<<"Depth bits: "<<glfwGetWindowParam(GLFW_DEPTH_BITS)<<std::endl;
	std::cout<<"Stencil bits: "<<glfwGetWindowParam(GLFW_STENCIL_BITS)<<std::endl;
	std::cout<<"FSAA samples: "<<glfwGetWindowParam(GLFW_FSAA_SAMPLES)<<std::endl;
	std::cout<<"Vendor: "<<glGetString(GL_VENDOR)<<std::endl;
	std::cout<<"Renderer: "<<glGetString(GL_RENDERER)<<std::endl;
	std::cout<<"Version: "<<glGetString(GL_VERSION)<<std::endl;
	std::cout<<"GLSL: "<<glGetString(GL_SHADING_LANGUAGE_VERSION)<<std::endl;

	return true;
}

void gl::destroyContext()
{
	glfwTerminate();
}

void gl::shutdown(const char *error)
{
	if(error != "")
	{
		std::cerr<<error<<std::endl;
		std::cin.get();
	}
	gl::destroyContext();
	exit(error != "" ? EXIT_FAILURE : EXIT_SUCCESS);
}