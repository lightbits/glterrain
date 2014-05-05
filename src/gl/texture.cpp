#include <gl/texture.h>
#include <SOIL.h>

const std::string TEXTURE_NOT_BOUND = "Texture not bound";
const Texture2D *Texture2D::bound = nullptr;

Texture2D::Texture2D() : 
	m_width(0), 
	m_height(0), 
	m_handle(0)
{ }

void Texture2D::dispose()
{
	glDeleteTextures(1, &m_handle);
	m_width  = 0;
	m_height = 0;
}

void Texture2D::create(
	GLint level,
	GLint internalFormat, 
	GLsizei width, 
	GLsizei height,
	GLenum format,
	GLenum type, 
	const GLvoid *data)
{
	dispose();
	glGenTextures(1, &m_handle);
	glBindTexture(GL_TEXTURE_2D, m_handle);
	glTexImage2D(GL_TEXTURE_2D, level, internalFormat, width, height, 0, format, type, data);
	glBindTexture(GL_TEXTURE_2D, 0);
	m_width  = width;
	m_height = height;
}

void Texture2D::update(
		const GLvoid *data,
		GLenum format,
		GLenum type,
		int xoffset,
		int yoffset,
		int width,
		int height,
		GLint level)
{
	if (bound != this)
		throw std::runtime_error("Texture not bound");

	if (width == 0)
		width = getWidth();

	if (height == 0)
		height = getHeight();

	glTexSubImage2D(GL_TEXTURE_2D, level, xoffset, yoffset, width, height, format, type, data);
}

void Texture2D::copyFromFramebuffer(			
		GLint level,			
		GLint xoffset,			
		GLint yoffset,			
		GLint x, GLint y,		
		GLsizei width_,			
		GLsizei height_)
{
	// http://www.opengl.org/sdk/docs/man/xhtml/glCopyTexSubImage2D.xml
	throw std::runtime_error("copyFromFramebuffer not implemented");
	//glBindTexture(GL_TEXTURE_2D, m_handle);
	//glCopyTexSubImage2D(GL_TEXTURE_2D, level, xoffset, yoffset, x, y, width_, height_);
	//glBindTexture(GL_TEXTURE_2D, 0);
}

bool Texture2D::loadFromFile(const char *filename)
{
	GLuint tex = SOIL_load_OGL_texture(
		filename, 
		SOIL_LOAD_RGBA,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y);

	if (tex == 0)
	{
		std::cerr << "Failed to load texture (" << filename << "): " << SOIL_last_result() << std::endl;
		return false;
	}

	// Retreive the dimensions
	dispose();
	glBindTexture(GL_TEXTURE_2D, tex);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &m_width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &m_height);
	glBindTexture(GL_TEXTURE_2D, 0);

	m_handle = tex;
	
	return true;
}

void Texture2D::setTexParameteri(GLenum minFilter, GLenum magFilter, GLenum wrapS, GLenum wrapT)
{
	if (bound != this)
		bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
}

vec2i  Texture2D::getSize()      const { return vec2i(m_width, m_height); }
int    Texture2D::getWidth()     const { return getSize().x; }
int    Texture2D::getHeight()    const { return getSize().y; }
GLuint Texture2D::getHandle()    const { return m_handle; }

void Texture2D::bind(GLint unit) const 
{
	bound = this;
	glActiveTexture(unit);
	glBindTexture(GL_TEXTURE_2D, m_handle);
}

void Texture2D::bind() const 
{
	bound = this;
	glBindTexture(GL_TEXTURE_2D, m_handle); 
}

void Texture2D::unbind() 
{
	// TODO: Remember active texture unit
	bound = nullptr;
	glBindTexture(GL_TEXTURE_2D, 0); 
}