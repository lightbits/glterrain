#include "slabops.h"

static ShaderProgram 
	shader_jacobi,
	shader_advect,
	shader_forces,
	shader_dye,
	shader_divergence,
	shader_project,
	shader_boundary;

static BufferObject 
	vbo_quad, 
	vbo_line_upper,
	vbo_line_lower,
	vbo_line_left,
	vbo_line_right;

bool loadSlabOps()
{
	if (!shader_jacobi.loadFromFile("./demo/30fluid2D/quad.vs", "./demo/30fluid2D/jacobi.fs") ||
		!shader_advect.loadFromFile("./demo/30fluid2D/quad.vs", "./demo/30fluid2D/advect.fs") ||
		!shader_forces.loadFromFile("./demo/30fluid2D/quad.vs", "./demo/30fluid2D/forces.fs") ||
		!shader_dye.loadFromFile("./demo/30fluid2D/quad.vs", "./demo/30fluid2D/dye.fs") ||
		!shader_divergence.loadFromFile("./demo/30fluid2D/quad.vs", "./demo/30fluid2D/divergence.fs") ||
		!shader_project.loadFromFile("./demo/30fluid2D/quad.vs", "./demo/30fluid2D/project.fs") ||
		!shader_boundary.loadFromFile("./demo/30fluid2D/quad.vs", "./demo/30fluid2D/boundary.fs"))
		return false;

	if (!shader_jacobi.linkAndCheckStatus() ||
		!shader_advect.linkAndCheckStatus() ||
		!shader_forces.linkAndCheckStatus() ||
		!shader_dye.linkAndCheckStatus() ||
		!shader_divergence.linkAndCheckStatus() ||
		!shader_project.linkAndCheckStatus() ||
		!shader_boundary.linkAndCheckStatus())
		return false;

	float quad[] = {
		-1.0f + 2.0f * DX, -1.0f + 2.0f * DX, 0.0f + DX, 0.0f + DX,
		 1.0f - 2.0f * DX, -1.0f + 2.0f * DX, 1.0f - DX, 0.0f + DX,
		 1.0f - 2.0f * DX,  1.0f - 2.0f * DX, 1.0f - DX, 1.0f - DX,
		 1.0f - 2.0f * DX,  1.0f - 2.0f * DX, 1.0f - DX, 1.0f - DX,
		-1.0f + 2.0f * DX,  1.0f - 2.0f * DX, 0.0f + DX, 1.0f - DX,
		-1.0f + 2.0f * DX, -1.0f + 2.0f * DX, 0.0f + DX, 0.0f + DX
	};

	float line_upper[] = { -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	float line_lower[] = { -1.0f,-1.0f, 0.0f, 0.0f, 1.0f,-1.0f, 1.0f, 0.0f };
	float line_left[]  = { -1.0f,-1.0f, 0.0f, 0.0f,-1.0f, 1.0f, 0.0f, 1.0f };
	float line_right[] = {  1.0f,-1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f };

	vbo_quad.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(quad), quad);
	vbo_line_upper.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(line_upper), line_upper);
	vbo_line_lower.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(line_lower), line_lower);
	vbo_line_left.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(line_left), line_left);
	vbo_line_right.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(line_right), line_right);

	return true;
}

void jacobi(
	RenderTexture &inputTexture,
	RenderTexture &outputTexture, // Output texture
	RenderTexture &sourceTexture, // B texture
	float alpha, float beta, // Iteration parameters
	Renderer &gfx, Context &ctx, float dt)
{
	outputTexture.begin();
	inputTexture.bindTexture(GL_TEXTURE0);
	sourceTexture.bindTexture(GL_TEXTURE1);
	gfx.beginCustomShader(shader_jacobi);
	gfx.setUniform("tex_input", 0);
	gfx.setUniform("tex_source", 1);
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
	float dissipation,
	Renderer &gfx, Context &ctx, float dt)
{
	outputTexture.begin();
	velocityTexture.bindTexture(GL_TEXTURE0);
	sourceTexture.bindTexture(GL_TEXTURE1);
	gfx.beginCustomShader(shader_advect);
	gfx.setUniform("tex_velocity", 0);
	gfx.setUniform("tex_source", 1);
	gfx.setUniform("dissipation", dissipation);
	gfx.setUniform("dt", dt);
	vbo_quad.bind();
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.drawVertexBuffer(GL_TRIANGLES, 6);
	outputTexture.end();
}

void addForces(
	RenderTexture &inputTexture,
	RenderTexture &outputTexture,
	vec2 mouse_pos, vec2 mouse_vel,
	Renderer &gfx, Context &ctx, float dt)
{
	outputTexture.begin();
	inputTexture.bindTexture(GL_TEXTURE0);
	gfx.beginCustomShader(shader_forces);
	gfx.setUniform("tex_velocity", 0);
	gfx.setUniform("mouse_pos", mouse_pos);
	gfx.setUniform("mouse_vel", mouse_vel);
	gfx.setUniform("dt", dt);
	vbo_quad.bind();
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.drawVertexBuffer(GL_TRIANGLES, 6);
	outputTexture.end();
}

void addDye(
	RenderTexture &inputTexture,
	RenderTexture &outputTexture,
	vec2 mouse_pos,
	Renderer &gfx, Context &ctx, float dt)
{
	outputTexture.begin();
	inputTexture.bindTexture(GL_TEXTURE0);
	gfx.beginCustomShader(shader_dye);
	gfx.setUniform("tex_dye", 0);
	gfx.setUniform("mouse_pos", mouse_pos);
	vbo_quad.bind();
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.drawVertexBuffer(GL_TRIANGLES, 6);
	outputTexture.end();
}

void calculateDivergence(
	RenderTexture &velocityTexture,
	RenderTexture &outputTexture,
	Renderer &gfx, Context &ctx, float dt)
{
	outputTexture.begin();
	velocityTexture.bindTexture(GL_TEXTURE0);
	gfx.beginCustomShader(shader_divergence);
	gfx.setUniform("tex_velocity", 0);
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
	Renderer &gfx, Context &ctx, float dt)
{
	outputTexture.begin();
	velocityTexture.bindTexture(GL_TEXTURE0);
	pressureTexture.bindTexture(GL_TEXTURE1);
	gfx.beginCustomShader(shader_project);
	gfx.setUniform("tex_velocity", 0);
	gfx.setUniform("tex_pressure", 1);
	gfx.setUniform("dx", DX);
	vbo_quad.bind();
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.drawVertexBuffer(GL_TRIANGLES, 6);
	outputTexture.end();	
}

void applyBoundaryConditions(
	RenderTexture &sourceTexture,
	RenderTexture &outputTexture,
	float factor,
	Renderer &gfx, Context &ctx, float dt)
{
	outputTexture.begin();
	sourceTexture.bindTexture(GL_TEXTURE0);
	gfx.beginCustomShader(shader_boundary);
	gfx.setUniform("tex_source", 0);
	gfx.setUniform("factor", factor);	

	vbo_line_upper.bind();
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.setUniform("offset", vec2(0.0f, -DX));
	gfx.drawVertexBuffer(GL_LINES, 2);

	vbo_line_lower.bind();
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.setUniform("offset", vec2(0.0f, DX));
	gfx.drawVertexBuffer(GL_LINES, 2);

	vbo_line_left.bind();
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.setUniform("offset", vec2(DX, 0.0f));
	gfx.drawVertexBuffer(GL_LINES, 2);

	vbo_line_right.bind();
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.setUniform("offset", vec2(-DX, 0.0f));
	gfx.drawVertexBuffer(GL_LINES, 2);
	outputTexture.end();
}