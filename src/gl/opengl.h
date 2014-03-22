#ifndef OPENGL_H
#define OPENGL_H
#include <glload/gl_3_1_comp.h> // OpenGL version 3.1, compatibility profile.
#include <glload/gll.hpp> // The C-style loading interface
#include <GL/glfw.h>
#include <iostream>
#include <app/log.h>

/* Converts a GLenum to a text error */
const char *getErrorMessage(GLenum code);

/* 
Polls for OpenGL errors and writes them to the log.
@return The number of errors
*/
int checkGLErrors(Log &log);

#endif