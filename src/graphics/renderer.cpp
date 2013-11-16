#include <graphics/renderer.h>

static Renderer *activeRenderer = nullptr;

Renderer *getActiveRenderer() { return activeRenderer; }
ShaderProgram *getActiveShader() { 
	if(activeRenderer) 
		return activeRenderer->getCurrentShaderProgram();
	else
		return nullptr;
}

void setActiveRenderer(Renderer *r) { activeRenderer = r; }

Renderer::Renderer() : currentShaderProgram(nullptr)
{

}

void Renderer::init()
{
	setActiveRenderer(this);
}

void Renderer::dispose()
{
	setActiveRenderer(nullptr);
}

void Renderer::enableDepthTest(GLenum depthFunc)
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(depthFunc);
	glDepthRange(0.0, 1.0);
}

void Renderer::disableDepthTest()
{
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
}

void Renderer::enableBlending(GLenum srcFactor, GLenum destFactor)
{

}

void Renderer::disableBlending()
{

}

void Renderer::enableCulling(GLenum cullFace, GLenum frontFaceOrientation)
{
	glEnable(GL_CULL_FACE);
	glCullFace(cullFace);
	glFrontFace(frontFaceOrientation);
}

void Renderer::disableCulling()
{

}

void Renderer::clearColorBuffer()
{
	glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::clearDepthBuffer()
{
	glClear(GL_DEPTH_BUFFER_BIT);
}

void Renderer::clearColorAndDepth()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::setClearColor(const Color &color)
{
	glClearColor(color.r, color.g, color.b, color.a);
}

void Renderer::setClearDepth(double depth)
{
	glClearDepth(depth);
}

void Renderer::beginCustomShader(ShaderProgram &sp)
{
	currentShaderProgram = &sp;
}

void Renderer::endCustomShader()
{
	currentShaderProgram = nullptr;
}

void Renderer::draw(MeshBuffer &mesh, GLenum drawMode)
{
	
}

void Renderer::draw(std::vector<MeshBuffer> &meshes)
{

}