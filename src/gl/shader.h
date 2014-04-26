#ifndef SHADER_H
#define SHADER_H
#include <gl/opengl.h>
#include <string>

class Shader
{
public:
	Shader();
	void dispose();

	bool loadFromSource(const std::string &src, GLenum type);
	bool loadFromFile(const std::string &filename, GLenum type);
	bool compileAndCheckStatus(const std::string &src);
	GLuint getHandle() const;
private:
	GLuint m_handle;
	GLenum m_type;
};

#endif