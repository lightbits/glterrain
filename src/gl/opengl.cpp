#include <gl/opengl.h>

const char *getErrorMessage(GLenum code)
{
	switch (code)
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

int checkGLErrors(Log &log)
{
	int num_errors = 0;
	GLenum error = glGetError();
	while (error != GL_NO_ERROR)
	{
		log << getErrorMessage(error) << '\n';
		num_errors++;
		error = glGetError();
	}
	return num_errors;
}