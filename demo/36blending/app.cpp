/*
Correct blending:

draw_scene:
	use_framebuffer(rt_geometry)
	depth_write(true)
	depth_test(true, GL_LEQUAL)
	clear_color(0, 0, 0, 1)
	clear_depth(1)
	blend_mode(one, one_minus_src_alpha)
	draw_geometry()

	use_framebuffer(rt_particles)
	color_mask(0, 0, 0, 0)
	draw_geometry()
	color_mask(1, 1, 1, 1)

	if front_to_back:
		blend_mode(one_minus_dst_alpha, one)
	else:
		blend_mode(one, one_minus_src_alpha)
	depth_write(false)
	draw_particles()

	use_framebuffer(0)
	depth_write(false)
	depth_test(false)
	clear_color(0, 0, 0, 1)
	blend_mode(one, one_minus_src_alpha)
	draw_texture(rt_geometry)
	draw_texture(rt_particles)
*/

#include "app.h"
using namespace transform;

VertexArray vao;
ShaderProgram shader;
MeshBuffer quad;
mat4 
	mat_projection, 
	mat_view;
Texture2D
	tex1,
	tex2,
	tex3;
RenderTexture
	rt_geometry,
	rt_sprites;

bool load()
{
	if (!shader.loadFromFile("./demo/36blending/quad"))
		return false;

	if (!shader.linkAndCheckStatus())
		return false;

	if (!tex1.loadFromFile("./demo/36blending/tex1.png") ||
		!tex2.loadFromFile("./demo/36blending/tex2.png") ||
		!tex3.loadFromFile("./demo/36blending/tex3.png"))
		return false;

	return true;
}

void free()
{
	quad.dispose();
	vao.dispose();
}

void init(Renderer &gfx, Context &ctx)
{	
	vao.create();
	vao.bind();

	quad = MeshBuffer(Mesh::genScreenSpaceTexQuad());
	mat_projection = glm::perspective(PI / 4.0f, ctx.getWidth() / (float)ctx.getHeight(), 0.1f, 10.0f);
	mat_view = translate(0.0f, 0.0f, -2.0f) * rotateX(-0.65f) * rotateY(0.65f);

	gfx.beginCustomShader(shader);
	gfx.setUniform("view", mat_view);
	gfx.setUniform("projection", mat_projection);
	gfx.setUniform("tex", 0);

	rt_geometry.create(ctx.getWidth(), ctx.getHeight());
	rt_sprites.create(ctx.getWidth(), ctx.getHeight());
}

void update(Renderer &gfx, Context &ctx, double dt)
{

}

void renderGeometry(Renderer &gfx, Context &ctx, double dt)
{

}

void renderCorrect(Renderer &gfx, Context &ctx, double dt)
{
	glDepthRange(0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glClearDepth(1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	tex1.bind();
	gfx.setUniform("model", scale(0.5f));
	gfx.setUniform("color", vec4(1.0f, 1.0f, 1.0f, 1.0f));
	quad.draw();

	glDepthMask(GL_FALSE);
	tex3.bind();
	gfx.setUniform("model", translate(0.0f, 0.0f, 0.25f) * scale(0.5f) * rotateY(-0.8f));
	gfx.setUniform("color", vec4(1.0f, 1.0f, 1.0f, 0.5f));
	quad.draw();

	tex2.bind();
	gfx.setUniform("model", translate(0.0f, 0.0f, 0.5f) * scale(0.5f));
	gfx.setUniform("color", vec4(1.0f, 1.0f, 1.0f, 0.5f));
	quad.draw();
}

void renderWeird(Renderer &gfx, Context &ctx, double dt)
{
	// Render geometry framebuffer back to front
	rt_geometry.begin();

	glEnable(GL_DEPTH_TEST);
	glDepthRange(0.0f, 1.0f);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glClearDepth(1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	tex1.bind();
	gfx.setUniform("model", scale(0.5f));
	gfx.setUniform("color", vec4(1.0f, 1.0f, 1.0f, 1.0f));
	quad.draw();
	rt_geometry.end();

	// Transfer depth-values to the sprite framebuffer
	//glBindFramebuffer(GL_READ_FRAMEBUFFER, rt_geometry.getFramebuffer().getHandle());
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rt_sprites.getFramebuffer().getHandle());
	//glBlitFramebuffer(0, 0, ctx.getWidth(), ctx.getHeight(), 0, 0, ctx.getWidth(), ctx.getHeight(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	//glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	// Render sprite framebuffer front to back
	rt_sprites.begin();
	glClearDepth(1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Render depth from geometry into this framebuffer for use in depth test with sprites
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	tex1.bind();
	gfx.setUniform("model", scale(0.5f));
	gfx.setUniform("color", vec4(1.0f, 1.0f, 1.0f, 1.0f));
	quad.draw();
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	// The sprites do not occlude eachother, so we disable depth writing (but keep depth test)
	glDepthMask(GL_FALSE);
	if (ctx.isKeyPressed('b'))
	{
		// Back to front
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		tex3.bind();
		gfx.setUniform("model", translate(0.0f, 0.0f, 0.25f) * scale(0.5f) * rotateY(-0.8f));
		gfx.setUniform("color", vec4(1.0f, 1.0f, 1.0f, 0.5f));
		quad.draw();

		tex2.bind();
		gfx.setUniform("model", translate(0.0f, 0.0f, 0.5f) * scale(0.5f));
		gfx.setUniform("color", vec4(1.0f, 1.0f, 1.0f, 0.5f));
		quad.draw();
	}
	else
	{
		// Front to back
		glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
		tex2.bind();
		gfx.setUniform("model", translate(0.0f, 0.0f, 0.5f) * scale(0.5f));
		gfx.setUniform("color", vec4(1.0f, 1.0f, 1.0f, 0.5f));
		quad.draw();

		tex3.bind();
		gfx.setUniform("model", translate(0.0f, 0.0f, 0.25f) * scale(0.5f) * rotateY(-0.8f));
		gfx.setUniform("color", vec4(1.0f, 1.0f, 1.0f, 0.5f));
		quad.draw();
	}
	rt_sprites.end();

	// Composite results
	glDisable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	gfx.setUniform("model", glm::inverse(mat_view) * glm::inverse(mat_projection));
	gfx.setUniform("color", vec4(1.0f));
	rt_geometry.bindTexture();
	quad.draw();
	rt_sprites.bindTexture();
	quad.draw();
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	if (ctx.isKeyPressed('s'))
		renderCorrect(gfx, ctx, dt);
	else
		renderWeird(gfx, ctx, dt);
}