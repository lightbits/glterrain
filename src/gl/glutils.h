#ifndef GLUTILS_H
#define GLUTILS_H
#include <glload/gl_3_1_comp.h> // OpenGL version 3.1, compatibility profile.
#include <glload/gll.hpp> // The C-style loading interface
#include <GL/glfw.h>
#include <iostream>

/* Converts a GLenum to a text error */
const char *getErrorMessage(GLenum code);

/* Polls for OpenGL errors and writes them to the output stream.
Returns true if there were errors. */
bool checkGLErrors(std::ostream &out);

#endif