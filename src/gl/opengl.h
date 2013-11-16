#ifndef OPENGL_H
#define OPENGL_H
//#include <glload/gl_3_1.h> // OpenGL version 3.1, core profile.
#include <glload/gl_3_1_comp.h> // OpenGL version 3.1, core profile.
#include <glload/gll.hpp> // The C-style loading interface
#include <GL/glfw.h> // Handles context

const int OPENGL_VERSION_MAJOR = 3;
const int OPENGL_VERSION_MINOR = 1;

const char *getErrorMessage(GLenum code);

bool createContext(const char *title, int x, int y, int w, int h, int depthbits, int stencilbits, int fsaa, bool fullscreen);
void destroyContext();
void shutdown(const char *error = "");

#endif