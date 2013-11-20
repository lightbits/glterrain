#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H
#include <gl/program.h>
#include <gl/shader.h>

class ShaderProgram
{
public:
	ShaderProgram();
	bool loadFromSource(const std::string &vertSrc, const std::string &fragSrc);
	bool loadFromFile(const std::string &vertName, const std::string &fragName);
	bool linkAndCheckStatus();
	void dispose();

	void begin();
	void end();

	// Binds the attribute with the given name to the given location
	void bindAttribute(GLuint location, const std::string &name);

	// Returns (a cached) uniform location for the given name
	GLint getUniformLocation(const std::string &name);

	// Returns (a cached) attribute location for the given name
	GLint getAttributeLocation(const std::string &name);

	/*
	stride: number of components between the first element in each attribute
	offset: number of components into the array the component is found
	note: not byte offset
	*/
	void setAttributefv(const std::string &name, GLsizei numComponents, GLsizei stride, GLsizei offset);
	void setAttributefv(GLuint location, GLsizei numComponents, GLsizei stride, GLsizei offset);
	/*void unsetAttribute(const std::string &name);
	void unsetAttribute(GLuint location);*/

	void setUniform(const std::string &name, const mat4 &mat);
	void setUniform(const std::string &name, const mat3 &mat);
	void setUniform(const std::string &name, const mat2 &mat);
	void setUniform(const std::string &name, const vec4 &vec);
	void setUniform(const std::string &name, const vec3 &vec);
	void setUniform(const std::string &name, const vec2 &vec);
	void setUniform(const std::string &name, GLfloat f);
	void setUniform(const std::string &name, GLint i);

	GLuint getProgram() const;
	GLuint getShader(GLenum type) const;
private:
	std::unordered_map<std::string, GLint> attribLocations;
	std::unordered_map<std::string, GLint> uniformLocations;
	Program program;
	Shader vertexShader;
	Shader fragmentShader;
};

#endif