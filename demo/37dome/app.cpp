#include "app.h"
#include <camera/fancycamera.h>
#define GLSL(src) "#version 150 core\n" #src

const char *res_shader_vs = GLSL(
	in vec3 position;

	uniform mat4 projection;
	uniform mat4 view;
	uniform mat4 model;

	out vec3 vBoxCoord;

	void main()
	{
		vBoxCoord = (model * vec4(position, 0.0)).xyz;
		vec4 viewPos = view * model * vec4(position, 0.0);
		viewPos.w = 1.0;
		gl_Position = projection * viewPos;
	}
);

const char *res_shader_fs = GLSL(
	in vec3 vBoxCoord;

	uniform sampler2D skydome;

	out vec4 outColor;

	void main()
	{
		vec3 v = normalize(vBoxCoord);
		float t = atan(v.z, v.x);
		float s = asin(v.y);

		t /= 3.1416; // pi
		s /= 1.5708; // pi / 2
		t = t * 0.5 + 0.5;
		s = s * 0.5 + 0.5;

		outColor = texture(skydome, vec2(t, s)) * 1.0;
		outColor += 0.0 * vec4(t, s, 0.0, 1.0);
	}
);

Texture2D tex_skydome;
ShaderProgram shader;
Model sphere;
MeshBuffer buffer_sphere;
VertexArray vao;
FancyCamera camera;

bool load()
{
	if (!shader.loadFromSource(res_shader_vs, res_shader_fs))
		return false;
	if (!shader.linkAndCheckStatus())
		return false;
	if (!tex_skydome.loadFromFile("./data/textures/aosky009_lite.png"))
		return false;
	return true;
}

void free()
{

}

void init(Renderer &gfx, Context &ctx)
{
	vao.create();
	vao.bind();

	buffer_sphere.create(Mesh::genUnitSphere(64, 64));
	sphere = Model(buffer_sphere);
	camera.reset(0.0f, 0.0f, vec3(0.0f, 0.2f, 1.0f));
}

void update(Renderer &gfx, Context &ctx, float dt)
{
	camera.update(gfx, ctx, dt);
}

void render(Renderer &gfx, Context &ctx, float dt)
{
	mat4 mat_projection = glm::perspective(PI / 4.0f, ctx.getWidth() / float(ctx.getHeight()), 0.1f, 10.0f);
	mat4 mat_view = camera.getViewMatrix();

	gfx.clear(0x00000000, 1.0);
	gfx.beginCustomShader(shader);
	gfx.setCullState(CullStates::CullNone);
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	tex_skydome.bind();
	gfx.setUniform("projection", mat_projection);
	gfx.setUniform("view", mat_view);
	gfx.setUniform("skydome", 0);
	sphere.transform = transform::scale(5.0f);
	sphere.draw();
	gfx.endCustomShader();
}

void keyPressed(int mod, SDL_Keycode key)
{

}

void keyReleased(int mod, SDL_Keycode key)
{

}

void mouseMoved(int x, int y, int dx, int dy)
{

}

void mouseDragged(int button, int x, int y, int dx, int dy)
{

}

void mousePressed(int button, int x, int y)
{

}

void mouseReleased(int button, int x, int y)
{

}