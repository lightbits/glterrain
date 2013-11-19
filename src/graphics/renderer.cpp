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

void Renderer::setDepthTestState(DepthTestState state) { depthTestState = state; state.set(); }

void Renderer::setCullState(CullState state) { cullState = state; state.set(); }

void Renderer::setRasterizerState(RasterizerState state) { rasterizerState = state; state.set(); }

void Renderer::setBlendState(BlendState state) { blendState = state; state.set(); }

void Renderer::enableUserStates() 
{
	cullState.set();
	depthTestState.set();
	rasterizerState.set();
	blendState.set();
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

void Renderer::setClearColor(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
}

void Renderer::setClearColor(const Color &color)
{
	setClearColor(color.r, color.g, color.b, color.a);
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

void Renderer::drawIndexedGeometry(GLenum drawMode, int indexCount, GLenum indexType)
{
	glDrawElements(drawMode, indexCount, indexType, 0);
}

void Renderer::draw(MeshBuffer &mesh, GLenum drawMode)
{
	
}

void Renderer::draw(std::vector<MeshBuffer> &meshes)
{

}

void Renderer::drawLine(const vec3 &v0, const vec3 &v1, const Color &color)
{
	BufferObject vbo;
	vbo.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
	vbo.bind();
	float data[] = { 
		v0.x, v0.y, v0.z, color.r, color.g, color.b, color.a, 
		v1.x, v1.y, v1.z, color.r, color.g, color.b, color.a
	};
	vbo.bufferData(2 * 7 * sizeof(float), data);
	currentShaderProgram->setAttributefv("position", 3, 7, 0);
	currentShaderProgram->setAttributefv("color", 3, 7, 3);
	glDrawArrays(GL_LINES, 0, 2);
	vbo.unbind();
	vbo.dispose();
}

void Renderer::drawLine(const vec3 &v0, const vec3 &v1, const Color &color, float thickness)
{
	BufferObject vbo;
	vbo.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
	vbo.bind();

	float length = glm::length(v1 - v0);
	float alpha = (v1.x - v0.x) / length;
	float beta = -(v1.y - v0.y) / length;
	vec3 offset = thickness * vec3(beta, alpha, 0.0f);
	vec3 a = v0 + offset;
	vec3 b = v1 + offset;
	vec3 c = v1 - offset;
	vec3 d = v0 - offset;
	float data[] = {
		a.x, a.y, a.z, color.r, color.g, color.b, color.a,
		d.x, d.y, d.z, color.r, color.g, color.b, color.a,
		c.x, c.y, c.z, color.r, color.g, color.b, color.a,
		c.x, c.y, c.z, color.r, color.g, color.b, color.a,
		b.x, b.y, b.z, color.r, color.g, color.b, color.a,
		a.x, a.y, a.z, color.r, color.g, color.b, color.a
	};
	vbo.bufferData(6 * 7 * sizeof(float), data);
	currentShaderProgram->setAttributefv("position", 3, 7, 0);
	currentShaderProgram->setAttributefv("color", 3, 7, 3);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	vbo.unbind();
	vbo.dispose();
}