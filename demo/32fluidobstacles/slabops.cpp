#include "slabops.h"

static ShaderProgram 
	shader_jacobi,
	shader_advect,
	shader_impulse,
	shader_buoyancy,
	shader_divergence,
	shader_project,
	shader_boundary;

static BufferObject vbo_quad;

bool loadSlabOps()
{
	if (!shader_jacobi.loadFromFile("./demo/32fluidobstacles/quad.vs", "./demo/32fluidobstacles/jacobi.fs") ||
		!shader_advect.loadFromFile("./demo/32fluidobstacles/quad.vs", "./demo/32fluidobstacles/advect.fs") ||
		!shader_impulse.loadFromFile("./demo/32fluidobstacles/quad.vs", "./demo/32fluidobstacles/impulse.fs") ||
		!shader_buoyancy.loadFromFile("./demo/32fluidobstacles/quad.vs", "./demo/32fluidobstacles/buoyancy.fs") ||
		!shader_divergence.loadFromFile("./demo/32fluidobstacles/quad.vs", "./demo/32fluidobstacles/divergence.fs") ||
		!shader_project.loadFromFile("./demo/32fluidobstacles/quad.vs", "./demo/32fluidobstacles/project.fs") ||
		!shader_boundary.loadFromFile("./demo/32fluidobstacles/quad.vs", "./demo/32fluidobstacles/boundary.fs"))
		return false;

	if (!shader_jacobi.linkAndCheckStatus() ||
		!shader_advect.linkAndCheckStatus() ||
		!shader_impulse.linkAndCheckStatus() ||
		!shader_buoyancy.linkAndCheckStatus() ||
		!shader_divergence.linkAndCheckStatus() ||
		!shader_project.linkAndCheckStatus() ||
		!shader_boundary.linkAndCheckStatus())
		return false;

	float quad[] = {
		-1.0f, -1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, 1.0f, 1.0f,
		 1.0f,  1.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f
	};

	vbo_quad.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(quad), quad);

	return true;
}

void jacobi(
	RenderTexture &inputTexture,
	RenderTexture &outputTexture, // Output texture
	RenderTexture &sourceTexture, // B texture
	RenderTexture &obstaclesTexture,
	float alpha, float beta, // Iteration parameters
	Renderer &gfx, Context &ctx, float dt)
{
	outputTexture.begin();
	inputTexture.bindTexture(GL_TEXTURE0);
	sourceTexture.bindTexture(GL_TEXTURE1);
	obstaclesTexture.bindTexture(GL_TEXTURE2);
	gfx.beginCustomShader(shader_jacobi);
	gfx.setUniform("tex_input", 0);
	gfx.setUniform("tex_source", 1);
	gfx.setUniform("tex_obstacles", 2);
	gfx.setUniform("alpha", alpha);
	gfx.setUniform("beta", beta);
	gfx.setUniform("dx", 1.0f / GRID_SIZE);
	vbo_quad.bind();
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.drawVertexBuffer(GL_TRIANGLES, 6);
	outputTexture.end();
}

void advect(
	RenderTexture &velocityTexture,
	RenderTexture &sourceTexture,
	RenderTexture &outputTexture,
	RenderTexture &obstaclesTexture,
	float dissipation,
	Renderer &gfx, Context &ctx, float dt)
{
	outputTexture.begin();
	velocityTexture.bindTexture(GL_TEXTURE0);
	sourceTexture.bindTexture(GL_TEXTURE1);
	obstaclesTexture.bindTexture(GL_TEXTURE2);
	gfx.beginCustomShader(shader_advect);
	gfx.setUniform("tex_velocity", 0);
	gfx.setUniform("tex_source", 1);
	gfx.setUniform("tex_obstacles", 2);
	gfx.setUniform("dissipation", dissipation);
	gfx.setUniform("dt", dt);
	vbo_quad.bind();
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.drawVertexBuffer(GL_TRIANGLES, 6);
	outputTexture.end();
}

void addBuocancy(
	RenderTexture &velocityTexture,
	RenderTexture &temperatureTexture,
	RenderTexture &outputTexture,
	Renderer &gfx, Context &ctx, float dt)
{
	outputTexture.begin();
	velocityTexture.bindTexture(GL_TEXTURE0);
	temperatureTexture.bindTexture(GL_TEXTURE1);
	gfx.beginCustomShader(shader_buoyancy);
	gfx.setUniform("tex_velocity", 0);
	gfx.setUniform("tex_temperature", 1);
	gfx.setUniform("ambient", AMBIENT);
	gfx.setUniform("sigma", SIGMA);
	gfx.setUniform("kappa", KAPPA);
	gfx.setUniform("dt", dt);
	vbo_quad.bind();
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.drawVertexBuffer(GL_TRIANGLES, 6);
	outputTexture.end();
}

void applyImpulse(
	RenderTexture &inputTexture,
	RenderTexture &outputTexture,
	vec2 position, float radius,
	float effect,
	Renderer &gfx, Context &ctx, float dt)
{
	outputTexture.begin();
	inputTexture.bindTexture(GL_TEXTURE0);
	gfx.beginCustomShader(shader_impulse);
	gfx.setUniform("tex_input", 0);
	gfx.setUniform("impulse_radius", radius);
	gfx.setUniform("impulse_effect", effect);
	gfx.setUniform("impulse_pos", position);
	vbo_quad.bind();
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.drawVertexBuffer(GL_TRIANGLES, 6);
	outputTexture.end();	
}

void calculateDivergence(
	RenderTexture &velocityTexture,
	RenderTexture &outputTexture,
	RenderTexture &obstaclesTexture,
	Renderer &gfx, Context &ctx, float dt)
{
	outputTexture.begin();
	velocityTexture.bindTexture(GL_TEXTURE0);
	obstaclesTexture.bindTexture(GL_TEXTURE1);
	gfx.beginCustomShader(shader_divergence);
	gfx.setUniform("tex_velocity", 0);
	gfx.setUniform("tex_obstacles", 1);
	gfx.setUniform("dx", DX);
	vbo_quad.bind();
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.drawVertexBuffer(GL_TRIANGLES, 6);
	outputTexture.end();		
}

void subtractGradient(
	RenderTexture &velocityTexture,
	RenderTexture &pressureTexture,
	RenderTexture &outputTexture,
	RenderTexture &obstaclesTexture,
	Renderer &gfx, Context &ctx, float dt)
{
	outputTexture.begin();
	velocityTexture.bindTexture(GL_TEXTURE0);
	pressureTexture.bindTexture(GL_TEXTURE1);
	obstaclesTexture.bindTexture(GL_TEXTURE2);
	gfx.beginCustomShader(shader_project);
	gfx.setUniform("tex_velocity", 0);
	gfx.setUniform("tex_pressure", 1);
	gfx.setUniform("tex_obstacles", 2);
	gfx.setUniform("dx", DX);
	vbo_quad.bind();
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.drawVertexBuffer(GL_TRIANGLES, 6);
	outputTexture.end();	
}