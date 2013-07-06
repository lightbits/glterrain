#ifndef SLGL_GRAPHICS_PROGRAM_H
#define SLGL_GRAPHICS_PROGRAM_H
#include <graphics/opengl.h>
#include <common/matrix.h>
#include <common/vec.h>
#include <string>

namespace graphics
{

class Program
{
public:
	Program();

	// Delete program and shader objects
	void dispose() const;

	void use() const;
	void unuse() const;

	// Creates a program using the given shaders
	void compile(const std::string &vertexSrc, const std::string &fragmentSrc);

	GLuint getHandle() const;
	GLint getAttribLocation(const GLchar *name) const;
	GLint getUniformLocation(const GLchar *name) const;

	/*
	Define and enable a vertex attribute.
	size: the number of components per vertex attribute
	type: the data type of each component
	stride: the byte offset between each consecutive vertex attributes
	offset: the byte offset of the first component of the first vertex attribute
	Specifies the index of the generic vertex attribute to be modified.
	*/
	void enableVertexAttrib(const GLchar *name, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *offset) const;
	void enableVertexAttrib(GLint location, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *offset) const;

	/*
	Binds the out variable of the given name to the given color number. This can either be done
	in-shader, by using layout (location = x) out vec4 diffusecolor,
	automatically, by letting OpenGL assign an arbitrary color number, or
	through pre-link specification, using this function before linking the program.
	*/
	void bindFragDataLocation(GLint colorNumber, const GLchar *name) const;

	/*
	Note that glUniform* requires that the program is in use.
	Consider using glProgramUniform* instead.
	*/

	void uniform(GLint location, const mat4 &mat) const;
	void uniform(GLint location, const mat3 &mat) const;
	void uniform(GLint location, const mat2 &mat) const;
	void uniform(GLint location, const vec4 &vec) const;
	void uniform(GLint location, const vec3 &vec) const;
	void uniform(GLint location, const vec2 &vec) const;
	void uniform(GLint location, GLfloat f) const;
	void uniform(GLint location, GLint i) const;

	void uniform(const GLchar *name, const mat4 &mat) const;
	void uniform(const GLchar *name, const mat3 &mat) const;
	void uniform(const GLchar *name, const mat2 &mat) const;
	void uniform(const GLchar *name, const vec4 &vec) const;
	void uniform(const GLchar *name, const vec3 &vec) const;
	void uniform(const GLchar *name, const vec2 &vec) const;
	void uniform(const GLchar *name, GLfloat f) const;
	void uniform(const GLchar *name, GLint i) const;
private:
	GLuint program;
	GLuint vertexShader;
	GLuint fragmentShader;

	//std::map<const GLchar*, GLint> shaderIndices;
};

}

#endif