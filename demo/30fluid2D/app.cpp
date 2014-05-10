#include "app.h"

struct Slab
{
	int input_index;
	int output_index;

	Slab() : input_index(0), output_index(1) { }

	void swapSurfaces()
	{
		input_index = output_index;
		output_index = (output_index + 1) % 2;
	}

	RenderTexture &ping() { return targets[input_index]; }
	RenderTexture &pong() { return targets[output_index]; }

	RenderTexture targets[2];
};

VertexArray vao;
ShaderProgram 
	shader_jacobi,
	shader_advect,
	shader_forces,
	shader_divergence,
	shader_project,
	shader_boundary,
	shader_dye,
	shader_texture;
BufferObject 
	vbo_quad, 
	vbo_line_upper,
	vbo_line_lower,
	vbo_line_left,
	vbo_line_right;

Slab velocity, pressure, dye;
RenderTexture divergence;
const int GRID_SIZE = 256;
const float DX = 1.0f / GRID_SIZE;

bool load()
{
	if (!shader_jacobi.loadFromFile("./demo/30fluid2D/quad.vs", "./demo/30fluid2D/jacobi.fs") ||
		!shader_advect.loadFromFile("./demo/30fluid2D/quad.vs", "./demo/30fluid2D/advect.fs") ||
		!shader_forces.loadFromFile("./demo/30fluid2D/quad.vs", "./demo/30fluid2D/forces.fs") ||
		!shader_divergence.loadFromFile("./demo/30fluid2D/quad.vs", "./demo/30fluid2D/divergence.fs") ||
		!shader_project.loadFromFile("./demo/30fluid2D/quad.vs", "./demo/30fluid2D/project.fs") ||
		!shader_boundary.loadFromFile("./demo/30fluid2D/quad.vs", "./demo/30fluid2D/boundary.fs") ||
		!shader_dye.loadFromFile("./demo/30fluid2D/quad.vs", "./demo/30fluid2D/dye.fs") ||
		!shader_texture.loadFromFile("./demo/30fluid2D/quad.vs", "./demo/30fluid2D/texture.fs"))
		return false;

	if (!shader_jacobi.linkAndCheckStatus() ||
		!shader_advect.linkAndCheckStatus() ||
		!shader_forces.linkAndCheckStatus() ||
		!shader_divergence.linkAndCheckStatus() ||
		!shader_project.linkAndCheckStatus() ||
		!shader_boundary.linkAndCheckStatus() ||
		!shader_dye.linkAndCheckStatus() ||
		!shader_texture.linkAndCheckStatus())
		return false;

	return true;
}

void free()
{
	vao.dispose();
}

void init(Renderer &gfx, Context &ctx)
{
	vao.create();
	vao.bind();

	velocity.ping().create(0, GL_RGB32F, GRID_SIZE, GRID_SIZE, GL_RG, GL_FLOAT, NULL);
	velocity.pong().create(0, GL_RGB32F, GRID_SIZE, GRID_SIZE, GL_RG, GL_FLOAT, NULL);
	pressure.ping().create(0, GL_RGB32F, GRID_SIZE, GRID_SIZE, GL_RG, GL_FLOAT, NULL);
	pressure.pong().create(0, GL_RGB32F, GRID_SIZE, GRID_SIZE, GL_RG, GL_FLOAT, NULL);
	divergence.create(0, GL_RGB32F, GRID_SIZE, GRID_SIZE, GL_RG, GL_FLOAT, NULL);
	dye.ping().create(0, GL_RGB32F, GRID_SIZE, GRID_SIZE, GL_RG, GL_FLOAT, NULL);
	dye.pong().create(0, GL_RGB32F, GRID_SIZE, GRID_SIZE, GL_RG, GL_FLOAT, NULL);

	velocity.ping().getColorBuffer().setTexParameteri(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	velocity.pong().getColorBuffer().setTexParameteri(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	pressure.ping().getColorBuffer().setTexParameteri(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	pressure.pong().getColorBuffer().setTexParameteri(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	divergence.getColorBuffer().setTexParameteri(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	dye.ping().getColorBuffer().setTexParameteri(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	dye.pong().getColorBuffer().setTexParameteri(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

	float quad_data[] = {
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

	vbo_quad.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(quad_data), quad_data);
	vbo_line_upper.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(line_upper), line_upper);
	vbo_line_lower.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(line_lower), line_lower);
	vbo_line_left.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(line_left), line_left);
	vbo_line_right.create(GL_ARRAY_BUFFER, GL_STATIC_DRAW, sizeof(line_right), line_right);
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
	Renderer &gfx, Context &ctx, float dt)
{
	outputTexture.begin();
	velocityTexture.bindTexture(GL_TEXTURE0);
	sourceTexture.bindTexture(GL_TEXTURE1);
	gfx.beginCustomShader(shader_advect);
	gfx.setUniform("tex_velocity", 0);
	gfx.setUniform("tex_source", 1);
	gfx.setUniform("dt", dt);
	vbo_quad.bind();
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.drawVertexBuffer(GL_TRIANGLES, 6);
	outputTexture.end();
}

void addForces(
	RenderTexture &velocityTexture,
	RenderTexture &outputTexture,
	Renderer &gfx, Context &ctx, float dt)
{
	outputTexture.begin();
	velocityTexture.bindTexture(GL_TEXTURE0);
	gfx.beginCustomShader(shader_forces);
	gfx.setUniform("tex_velocity", 0);
	gfx.setUniform("time", ctx.getElapsedTime());
	gfx.setUniform("dt", dt);
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
	gfx.setUniform("dx", 1.0f / GRID_SIZE);
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
	gfx.setUniform("dx", 1.0f / GRID_SIZE);
	vbo_quad.bind();
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.drawVertexBuffer(GL_TRIANGLES, 6);
	outputTexture.end();	
}

void applyBoundaryCondition(
	RenderTexture &sourceTexture,
	RenderTexture &outputTexture,
	BufferObject &vbo,
	const vec2 &offset,
	float scale,
	Renderer &gfx, Context &ctx, float dt)
{
	outputTexture.begin();
	sourceTexture.bindTexture(GL_TEXTURE0);
	gfx.beginCustomShader(shader_boundary);
	gfx.setUniform("tex_source", 0);
	gfx.setUniform("offset", offset);
	gfx.setUniform("scale", scale);
	vbo.bind();
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.drawVertexBuffer(GL_LINES, 2);
	outputTexture.end();
}

vec2i last_mouse_pos;
void handleMouseEvent(int x, int y, Renderer &gfx, Context &ctx, float dt)
{
	float u = (float)x / ctx.getWidth();
	float v = 1.0f - (float)y / ctx.getHeight();
	dye.pong().begin();
	dye.ping().bindTexture(GL_TEXTURE0);
	gfx.beginCustomShader(shader_dye);
	gfx.setUniform("tex_dye", 0);
	gfx.setUniform("mouse_pos", vec2(u, v));
	vbo_quad.bind();
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.drawVertexBuffer(GL_TRIANGLES, 6);
	dye.pong().end();
}

void updateForces(Renderer &gfx, Context &ctx, float dt)
{
	vec2i mouse_pos = ctx.getMousePos();

	if (ctx.isMousePressed(SDL_BUTTON_LEFT))
	{
		// Mouse position in texel coordinates
		vec2 p = vec2(mouse_pos.x, ctx.getHeight() - mouse_pos.y);
		p.x /= (float)ctx.getWidth();
		p.y /= (float)ctx.getHeight();

		// Mouse velocity in texel coordinates
		vec2 v = mouse_pos - last_mouse_pos;
		v.x /= (float)ctx.getWidth();
		v.y /= (float)ctx.getHeight();

		velocity.pong().begin();
		velocity.ping().bindTexture(GL_TEXTURE0);
		gfx.beginCustomShader(shader_forces);
		gfx.setUniform("tex_velocity", 0);
		gfx.setUniform("mouse_pos", p);
		gfx.setUniform("mouse_vel", v);
		gfx.setUniform("dt", dt);
		vbo_quad.bind();
		gfx.setAttributefv("position", 2, 4, 0);
		gfx.setAttributefv("texel", 2, 4, 2);
		gfx.drawVertexBuffer(GL_TRIANGLES, 6);
		velocity.pong().end();
		velocity.swapSurfaces();
	}

	last_mouse_pos = mouse_pos;
}

void update(Renderer &gfx, Context &ctx, float dt)
{
	glViewport(0, 0, GRID_SIZE, GRID_SIZE);

	if (ctx.isMousePressed(SDL_BUTTON_LEFT))
	{
		handleMouseEvent(ctx.getMouseX(), ctx.getMouseY(), gfx, ctx, dt);
		dye.swapSurfaces();
	}

	advect(velocity.ping(), dye.ping(), dye.pong(), gfx, ctx, dt);
	dye.swapSurfaces();

	advect(velocity.ping(), velocity.ping(), velocity.pong(), gfx, ctx, dt);
	velocity.swapSurfaces();

	updateForces(gfx, ctx, dt);
	//addForces(velocity.ping(), velocity.pong(), gfx, ctx, dt);

	calculateDivergence(velocity.ping(), divergence, gfx, ctx, dt);

	// Clear pressure field (initial guess is 0)
	pressure.ping().begin();
	gfx.setClearColor(0.0f, 0.0f, 0.0f);
	gfx.clearColorAndDepth();
	pressure.ping().end();

	// Calculate pressure field
	for (int i = 0; i <= 200; ++i)
	{
		jacobi(pressure.ping(), pressure.pong(), divergence, -(DX * DX), 4.0f, gfx, ctx, dt);
		pressure.swapSurfaces();
	}

	// Project
	subtractGradient(velocity.ping(), pressure.ping(), velocity.pong(), gfx, ctx, dt);
	velocity.swapSurfaces();

	applyBoundaryCondition(velocity.ping(), velocity.pong(), vbo_line_lower, vec2(0.0, DX),  -1.0f, gfx, ctx, dt);
	applyBoundaryCondition(velocity.ping(), velocity.pong(), vbo_line_upper, vec2(0.0, -DX), -1.0f, gfx, ctx, dt);
	applyBoundaryCondition(velocity.ping(), velocity.pong(), vbo_line_left,  vec2(DX, 0.0),  -1.0f, gfx, ctx, dt);
	applyBoundaryCondition(velocity.ping(), velocity.pong(), vbo_line_right, vec2(-DX, 0.0), -1.0f, gfx, ctx, dt);
	applyBoundaryCondition(pressure.ping(), pressure.pong(), vbo_line_lower, vec2(0.0, DX),  1.0f, gfx, ctx, dt);
	applyBoundaryCondition(pressure.ping(), pressure.pong(), vbo_line_upper, vec2(0.0, -DX), 1.0f, gfx, ctx, dt);
	applyBoundaryCondition(pressure.ping(), pressure.pong(), vbo_line_left,  vec2(DX, 0.0),  1.0f, gfx, ctx, dt);
	applyBoundaryCondition(pressure.ping(), pressure.pong(), vbo_line_right, vec2(-DX, 0.0), 1.0f, gfx, ctx, dt);
}

void render(Renderer &gfx, Context &ctx, float dt)
{
	glViewport(0, 0, ctx.getWidth(), ctx.getHeight());
	vbo_quad.bind();
	dye.pong().bindTexture(GL_TEXTURE0);
	gfx.beginCustomShader(shader_texture);
	gfx.setClearColor(0.0f, 0.0f, 0.0f);
	gfx.clearColorAndDepth();
	gfx.setUniform("tex", 0);
	gfx.setAttributefv("position", 2, 4, 0);
	gfx.setAttributefv("texel", 2, 4, 2);
	gfx.drawVertexBuffer(GL_TRIANGLES, 6);
}