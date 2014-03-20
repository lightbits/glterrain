#ifndef GL_RESOURCE
#define GL_RESOURCE
#include <app/glcontext.h>

// Base class for classes that require an OpenGL context
class GLResource
{
public:
	GLResource()
	{
		if(!getActiveContext())
			throw std::runtime_error("Creating a GLResource requires an active context");
	}

	virtual ~GLResource() 
	{
		if(!getActiveContext())
			throw std::runtime_error("Destroying a GLResource requires an active context");
	}
};

#endif