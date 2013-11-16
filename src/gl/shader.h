#ifndef SHADER_H
#define SHADER_H
#include <gl/opengl.h>
#include <string>

class Shader
{
public:
	Shader(GLenum shaderType_);

	void dispose();
	void create();
	bool loadFromFile(const std::string &filename);
	bool compileAndCheckStatus(const std::string &src);
	GLuint getHandle() const;
private:
	GLuint shader;
	GLenum shaderType;
};

#endif