#ifndef OPENGL_H
#define OPENGL_H
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
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