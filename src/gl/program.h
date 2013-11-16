#ifndef PROGRAM_H
#define PROGRAM_H
#include <gl/opengl.h>
#include <gl/shader.h>
#include <common/matrix.h>
#include <common/vec.h>
#include <string>
#include <unordered_map>
#include <vector>

// Holds precomputed attrib/uniform locations
class ProgramLayout
{
public:
	void setAttrib(const std::string &name, GLint attrib);
	void setUniform(const std::string &name, GLint uniform);
	GLint getAttribLoc(const std::string &name) const;
	GLint getUniformLoc(const std::string &name) const;
private:
	std::unordered_map<std::string, GLint> attribs;
	std::unordered_map<std::string, GLint> uniforms;
};

class Program
{
public:
	Program();

	void dispose() const;
	void create();
	bool linkAndCheckStatus(const std::vector<Shader> &shaders);
	bool linkAndCheckStatus(const Shader &vertexShader, const Shader &fragmentShader);

	void use() const;
	void unuse() const;

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
private:
	GLuint program;
};

#endif