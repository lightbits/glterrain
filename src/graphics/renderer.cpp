#include <graphics/renderer.h>
static const std::string NO_ACTIVE_SHADER = "No active shader program";
static const std::string NO_ACTIVE_RENDERER = "No active renderer";
static Renderer *activeRenderer = nullptr;

Renderer *getActiveRenderer() 
{
	if (activeRenderer == nullptr)
		throw std::runtime_error(NO_ACTIVE_RENDERER);

	return activeRenderer; 
}

void setActiveRenderer(Renderer *r) { activeRenderer = r; }

ShaderProgram *getActiveShader() {
	return getActiveRenderer()->getCurrentShaderProgram();
}

Renderer::Renderer() : currentShaderProgram(nullptr)
{

}

Renderer::~Renderer()
{
	currentShaderProgram = nullptr;
}

void Renderer::init(Context &ctx)
{
	setActiveRenderer(this);
	int w, h;
	ctx.getSize(&w, &h);
	glViewport(0, 0, w, h);
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
	sp.begin();
}

void Renderer::endCustomShader()
{
	if (currentShaderProgram != nullptr)
		currentShaderProgram->end();
	currentShaderProgram = nullptr;
}

void Renderer::drawVertexBuffer(GLenum drawMode, int indexCount)
{
	if (currentShaderProgram == nullptr)
		throw std::runtime_error(NO_ACTIVE_SHADER);
	glDrawArrays(drawMode, 0, indexCount);
}

void Renderer::drawIndexedVertexBuffer(GLenum drawMode, int indexCount, GLenum indexType)
{
	if (currentShaderProgram == nullptr)
		throw std::runtime_error(NO_ACTIVE_SHADER);
	glDrawElements(drawMode, indexCount, indexType, 0);
}

void Renderer::draw(Mesh &mesh, GLenum drawMode)
{
	MeshBuffer buffer(mesh);
	buffer.draw();
}

void Renderer::draw(MeshBuffer &mesh, GLenum drawMode)
{
	if (currentShaderProgram == nullptr)
		throw std::runtime_error(NO_ACTIVE_SHADER);

	//mesh.vbo.bind();
	//mesh.ibo.bind();
	//int offset = 0;

	//setAttributefv("position", 3, 0, offset); 
	//offset += mesh.positionCount * 3;

	//if (mesh.normalCount > 0)
	//{
	//	setAttributefv("normal", 3, 0, offset); 
	//	offset += mesh.normalCount * 3;
	//}
	//if (mesh.colorCount > 0)
	//{
	//	setAttributefv("color", 4, 0, offset); 
	//	offset += mesh.colorCount * 4;
	//}
	//if (mesh.texelCount > 0)
	//{
	//	setAttributefv("texel", 2, 0, offset); 
	//}

	//glDrawElements(drawMode, mesh.indexCount, GL_UNSIGNED_INT, 0);
	//mesh.vbo.unbind();
	//mesh.ibo.unbind();
}

void Renderer::draw(std::vector<MeshBuffer> &meshes)
{

}

void Renderer::draw(Model &model)
{
	if (model.mesh == nullptr)
		throw std::runtime_error("Model has no associated mesh buffer");

	if (currentShaderProgram == nullptr)
		throw std::runtime_error(NO_ACTIVE_SHADER);

	setUniform("model", model.transform.top());
	model.mesh->draw();
}

void Renderer::drawQuad(float x, float y, float w, float h)
{
	// This is suspicious on AMD...?
	BufferObject vbo;
	vbo.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
	vbo.bind();
	float data[] = {
		x,     y,
		x + w, y,
		x + w, y + h,
		x + w, y + h,
		x,     y + h,
		x,     y
	};
	vbo.bufferData(sizeof(data), data);
	currentShaderProgram->setAttributefv("position", 2, 0, 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	vbo.dispose();
}

void Renderer::drawQuad(float x, float y, float w, float h, float z)
{
	BufferObject vbo;
	vbo.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
	vbo.bind();
	float data[] = {
		x,     y, z,
		x + w, y, z,
		x + w, y + h, z,
		x + w, y + h, z,
		x,     y + h, z,
		x,     y, z
	};
	vbo.bufferData(sizeof(data), data);
	currentShaderProgram->setAttributefv("position", 3, 0, 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	vbo.dispose();
}

void Renderer::drawTexQuad(float x, float y, float w, float h)
{
	BufferObject vbo;
	vbo.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
	vbo.bind();
	float data[] = {
		x,     y,     0.0f, 0.0f,
		x + w, y,     1.0f, 0.0f,
		x + w, y + h, 1.0f, 1.0f,
		x + w, y + h, 1.0f, 1.0f,
		x,     y + h, 0.0f, 1.0f,
		x,     y,     0.0f, 0.0f
	};
	vbo.bufferData(sizeof(data), data);
	currentShaderProgram->setAttributefv("position", 2, 4, 0);
	currentShaderProgram->setAttributefv("texel", 2, 4, 2);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	vbo.dispose();
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

GLint Renderer::getUniformLocation(const std::string &name)
{
	if (currentShaderProgram == nullptr)
		throw std::runtime_error("No active shader program");
	return currentShaderProgram->getUniformLocation(name);
}

GLint Renderer::getAttributeLocation(const std::string &name)
{
	if (currentShaderProgram == nullptr)
		throw std::runtime_error("No active shader program");
	return currentShaderProgram->getAttributeLocation(name);
}

void Renderer::bindAttribute(GLuint location, const std::string &name)
{
	if (currentShaderProgram == nullptr)
		throw std::runtime_error("No active shader program");
	currentShaderProgram->bindAttribute(location, name);
}

void Renderer::setAttributefv(const std::string &name, GLsizei numComponents, GLsizei stride, GLsizei offset)
{
	if (currentShaderProgram == nullptr)
		throw std::runtime_error("No active shader program");
	currentShaderProgram->setAttributefv(name, numComponents, stride, offset);
}

void Renderer::setAttributefv(GLint location, GLsizei numComponents, GLsizei stride, GLsizei offset)
{
	if (currentShaderProgram == nullptr)
		throw std::runtime_error("No active shader program");
	currentShaderProgram->setAttributefv(location, numComponents, stride, offset);
}

void Renderer::setUniform(const std::string &name, const mat4 &mat)
{
	if (currentShaderProgram == nullptr)
		throw std::runtime_error("No active shader program");
	currentShaderProgram->setUniform(name, mat);
}

void Renderer::setUniform(const std::string &name, const mat3 &mat)
{
	if (currentShaderProgram == nullptr)
		throw std::runtime_error("No active shader program");
	currentShaderProgram->setUniform(name, mat);
}

void Renderer::setUniform(const std::string &name, const mat2 &mat)
{
	if (currentShaderProgram == nullptr)
		throw std::runtime_error("No active shader program");
	currentShaderProgram->setUniform(name, mat);
}

void Renderer::setUniform(const std::string &name, const Color &color)
{
	if (currentShaderProgram == nullptr)
		throw std::runtime_error("No active shader program");
	currentShaderProgram->setUniform(name, vec4(color.r, color.g, color.b, color.a));
}

void Renderer::setUniform(const std::string &name, const vec4 &vec)
{
	if (currentShaderProgram == nullptr)
		throw std::runtime_error("No active shader program");
	currentShaderProgram->setUniform(name, vec);
}

void Renderer::setUniform(const std::string &name, const vec3 &vec)
{
	if (currentShaderProgram == nullptr)
		throw std::runtime_error("No active shader program");
	currentShaderProgram->setUniform(name, vec);
}

void Renderer::setUniform(const std::string &name, const vec2 &vec)
{
	if (currentShaderProgram == nullptr)
		throw std::runtime_error("No active shader program");
	currentShaderProgram->setUniform(name, vec);
}

void Renderer::setUniform(const std::string &name, GLdouble d)
{
	if (currentShaderProgram == nullptr)
		throw std::runtime_error("No active shader program");
	currentShaderProgram->setUniform(name, d);
}

void Renderer::setUniform(const std::string &name, GLfloat f)
{
	if (currentShaderProgram == nullptr)
		throw std::runtime_error("No active shader program");
	currentShaderProgram->setUniform(name, f);
}

void Renderer::setUniform(const std::string &name, GLint i)
{
	if (currentShaderProgram == nullptr)
		throw std::runtime_error("No active shader program");
	currentShaderProgram->setUniform(name, i);
}
