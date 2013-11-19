#include <iostream>
#include <fstream>
#include <vector>

#include <common/helpers.h>
#include <common/text.h>
#include <common/timer.h>
#include <common/vec.h>
#include <common/matrix.h>
#include <common/matrixstack.h>

#include <camera/camera.h>

#include <gl/opengl.h>
#include <gl/texture.h>
#include <gl/program.h>
#include <gl/bufferobject.h>
#include <gl/vertexformat.h>
#include <gl/vertexarray.h>
#include <gl/bufferedmesh.h>
#include <gl/shaderprogram.h>
#include <gl/framebuffer.h>
#include <graphics/renderer.h>
#include <graphics/color.h>
#include <graphics/trimesh.h>
#include <graphics/spritebatch.h>
#include <app/glcontext.h>

int main()
{
	const int windowWidth = 640;
	const int windowHeight = 480;
	GLContext context;
	if(!context.create("Heightmap Voxel Renderer", VideoMode(windowWidth, windowHeight, 24, 8, 8, false)))
		crash("Failed to create context");

	Renderer renderer;
	renderer.init();

	ShaderProgram 
		voxelShader,
		fboShader;

	if(!voxelShader.loadFromFile("data/shaders/voxel.vert", "data/shaders/voxel.frag") ||
		!fboShader.loadFromFile("data/shaders/fbopass.vert", "data/shaders/fbopass.frag"))
		crash("Failed to load resources");

	Texture heightmap;
	if(!heightmap.loadFromFile("data/img/heightmap1.png"))
		crash("Failed to load textures");
	heightmap.bind();
	heightmap.setTexParameteri(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	heightmap.unbind();

	voxelShader.linkAndCheckStatus();
	fboShader.linkAndCheckStatus();

	// Whatever-array-object
	VertexArray vao;
	vao.create();
	vao.bind();

	// Data for a fullscreen quad
	GLfloat voxelQuadVertexData[] = {
		-1.0f, -1.0f, 
		-1.0f, +1.0f,
		+1.0f, +1.0f,
		+1.0f, -1.0f
	};

	GLushort voxelQuadIndexData[] = { 
		0, 1, 2, 
		2, 3, 0 
	};

	// Data for a fullscreen textured quad
	GLfloat postEffectVertexData[] = {
	//	x		y		u		v
		-1.0f, -1.0f,	0.0f, 0.0f,
		-1.0f, +1.0f,	0.0f, 1.0f,
		+1.0f, +1.0f,	1.0f, 1.0f,
		+1.0f, -1.0f,	1.0f, 0.0f
	};

	GLushort postEffectIndexData[] = {
		0, 1, 2, 
		2, 3, 0
	};

	BufferObject postEffectVbo, postEffectIbo;
	postEffectVbo.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(GLfloat)*4*4, postEffectVertexData);
	postEffectIbo.create(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(GLushort)*3*2, postEffectIndexData);

	BufferObject voxelVbo, voxelIbo;
	voxelVbo.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(GLfloat) * 2 * 4, voxelQuadVertexData);
	voxelIbo.create(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(GLushort)*3*2, voxelQuadIndexData);

	const int fboSize = 64;

	// Create a render texture
	Texture rt;
	rt.create2d(0, GL_RGBA8, fboSize, fboSize, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	rt.bind();
	rt.setTexParameteri(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	rt.unbind();

	// Create a render buffer to store depth info
	Renderbuffer rbo;
	rbo.create();
	rbo.bind();
	rbo.storage(GL_DEPTH_COMPONENT, fboSize, fboSize);
	rbo.unbind();

	// Create a framebuffer to contain a depth and colortexture component
	Framebuffer fbo;
	fbo.create();
	fbo.bind();
	fbo.attachTexture2D(GL_COLOR_ATTACHMENT0, rt, 0);
	fbo.attachRenderbuffer(GL_DEPTH_ATTACHMENT, rbo);
	GLenum status = fbo.checkStatus();
	if(status != GL_FRAMEBUFFER_COMPLETE)
		crash("Framebuffer not complete.");
	fbo.unbind();

	renderer.enableDepthTest(GL_LEQUAL);
	renderer.setClearDepth(1.0);
	renderer.setClearColor(Color(0.55f, 0.45f, 0.45f, 1.0f));

	Timer timer;
	timer.start();
	double renderTime = 0.0;
	bool reloading = false;

	while(context.isOpen())
	{
		timer.step();
		float time = timer.getElapsedTime();
		double renderStart = timer.getElapsedTime();

		if(glfwGetKey('R') && !reloading)
		{
			reloading = true;
			if(!voxelShader.loadFromFile("data/shaders/voxel.vert", "data/shaders/voxel.frag") ||
				!fboShader.loadFromFile("data/shaders/fbopass.vert", "data/shaders/fbopass.frag"))
				context.close();

			voxelShader.linkAndCheckStatus();
			fboShader.linkAndCheckStatus();
		}
		else if(!glfwGetKey('R') && reloading)
		{
			reloading = false;
		}

		fbo.bind();
		glViewport(0, 0, fboSize, fboSize);
		renderer.clearColorAndDepth();
		voxelShader.begin();
		voxelVbo.bind();
		voxelIbo.bind();
		voxelShader.setAttributefv("position", 2, 0, 0);
		voxelShader.setUniform("heightmap", 0);
		voxelShader.setUniform("time", time);
		heightmap.bind();
		renderer.drawIndexedGeometry(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT);
		heightmap.unbind();
		voxelVbo.unbind();
		voxelIbo.unbind();
		voxelShader.end();

		fbo.unbind();

		glViewport(0, 0, windowWidth, windowHeight);
		renderer.clearColorAndDepth();
		fboShader.begin();
		postEffectVbo.bind();
		postEffectIbo.bind();
		fboShader.setAttributefv("position", 2, 4, 0);
		fboShader.setAttributefv("texel", 2, 4, 2);
		fboShader.setUniform("tex", 0);
		rt.bind();
		renderer.drawIndexedGeometry(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT);
		rt.unbind();
		postEffectVbo.unbind();
		postEffectIbo.unbind();
		fboShader.end();

		context.display();
		renderTime = timer.getElapsedTime() - renderStart;
		if(renderTime < 0.013)
			context.sleep(0.013 - renderTime);

		GLenum error = glGetError();
		if(error != GL_NO_ERROR)
		{
			std::cerr<<getErrorMessage(error)<<"...";
			std::cin.get();
			context.close();
		}
	}

	voxelShader.dispose();
	fboShader.dispose();
	rt.dispose();
	heightmap.dispose();
	voxelVbo.dispose();
	voxelIbo.dispose();
	postEffectVbo.dispose();
	postEffectIbo.dispose();
	vao.dispose();
	exit(EXIT_SUCCESS);
}