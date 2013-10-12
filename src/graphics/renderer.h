#ifndef SLGL_GRAPHICS_RENDERER_H
#define SLGL_GRAPHICS_RENDERER_H
#include <graphics/opengl.h>
#include <graphics/bufferedmesh.h>
#include <graphics/spritebatch.h>
#include <graphics/program.h>
#include <graphics/color.h>

namespace graphics
{

struct VideoMode
{
	VideoMode();
	VideoMode(int windowX, int windowY, 
		int windowWidth, int windowHeight, 
		int depthBits, int stencilBits, 
		int fsaaSamples, bool fullscreen);
	VideoMode(int windowWidth, int windowHeight, 
		int depthBits, int stencilBits, 
		int fsaaSamples, bool fullscreen);

	int x, y, w, h, db, sb, fsaa;
	bool fs;
};

class Renderer /* GLrenderer : public Renderer */
{
public:
	Renderer();

	/* Requests a new OpenGL context and loads GL functions */
	bool createContext(const std::string &title, const VideoMode &vm);
	void closeContext();
	void dispose();

	bool shouldContextClose() const;

	/* Sets the program that will be used when rendering */
	void setProgram(const Program &program, const ProgramLayout programLayout);

	void setUniform(const std::string &name, const mat4 &mat) const;
	void setUniform(const std::string &name, const mat3 &mat) const;
	void setUniform(const std::string &name, const mat2 &mat) const;
	void setUniform(const std::string &name, const vec4 &vec) const;
	void setUniform(const std::string &name, const vec3 &vec) const;
	void setUniform(const std::string &name, const vec2 &vec) const;
	void setUniform(const std::string &name, GLfloat f) const;
	void setUniform(const std::string &name, GLint i) const;

	/* Enables the checking of pixels with a depth test criterion.
	near, far: specifies a remapping of the depth value in NDC to window coordinates
	*/
	void enableDepthTest(GLenum depthFunc, GLfloat near, GLfloat far);
	void disableDepthTest();

	void enableBlending(GLenum srcFactor, GLenum destFactor);
	void disableBlending();

	void enableCulling(GLenum cullFace, GLenum frontFaceOrientation);
	void disableCulling();

	void clear(GLbitfield mask);
	void setClearColor(const Color &color);
	void setClearDepth(float depth);

	/* Renders the buffer data using the currently set program */
	void draw(const BufferedMesh &mesh);

	/* Renders a set of meshes */
	void draw(const std::vector<BufferedMesh> &meshes);

	/* Swap buffers */
	void display();

	/* Sets the context-thread to sleep for a given number of milliseconds */
	//void sleep(unsigned int ms);

	/* Returns the time between the last two successive calls to display()*/
	//float getDeltaTime() const;

	/* Returns the immediate fps calculated from the deltatime */
	//float getFps() const;

	/* Check for errors and prints to stream*/
	void pollError(std::ostream &out);
private:
	Color clearColor;
	GLfloat clearDepth;
	Program *currentProgram;
	ProgramLayout *currentLayout;
};

}

#endif