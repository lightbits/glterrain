#ifndef GLUTILS_H
#define GLUTILS_H
#include <glload/gl_3_1_comp.h> // OpenGL version 3.1, compatibility profile.
#include <glload/gll.hpp> // The C-style loading interface
#include <GL/glfw.h>

const char *getErrorMessage(GLenum code);

#endif