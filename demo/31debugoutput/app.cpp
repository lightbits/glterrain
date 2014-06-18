#include "app.h"

//VertexArray vao;
ShaderProgram shader;
Mesh cube_mesh;
Model cube;
MeshBuffer cube_buffer;

void CALLBACK errorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *msg, GLvoid *data)
{
	APP_LOG << "An OpenGL error occurred: " << msg << '\n';
	exit(EXIT_FAILURE);
}

bool load()
{
	if (!shader.loadFromFile("./demo/31debugoutput/simple"))
		return false;

	if (!shader.linkAndCheckStatus())
		return false;

	if (GL_KHR_debug)
		APP_LOG << "KHR_debug available\n";

	GLint v;
	glGetIntegerv(GL_CONTEXT_FLAGS, &v);
	if (v & GL_CONTEXT_FLAG_DEBUG_BIT)
		APP_LOG << "Debug context present\n";

	glDebugMessageCallback(errorCallback, NULL);

	return true;
}

void free()
{
	cube_buffer.dispose();
	//vao.dispose();
}

void init(Renderer &gfx, Context &ctx)
{	
	//vao.create();
	//vao.bind();

	Mesh cube_mesh = Mesh::genUnitCube(true, false, true);
	cube_buffer.create(cube_mesh);
	cube = Model(cube_buffer);
}

void update(Renderer &gfx, Context &ctx, double dt)
{

}

void render(Renderer &gfx, Context &ctx, double dt)
{
	// Invalid parameter
	//glEnable(GL_UNIFORM_BUFFER);

	// No VAO is bound!
	gfx.setClearDepth(1.0);
	gfx.setClearColor(0.2f, 0.2f, 0.3f);
	gfx.clearColorAndDepth();
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	gfx.setCullState(CullStates::CullNone);

	gfx.beginCustomShader(shader);
	gfx.setUniform("view", transform::translate(0.0f, 0.0f, -3.5f));
	gfx.setUniform("projection", glm::perspective(45.0f, ctx.getWidth() / (float)ctx.getHeight(), 0.05f, 10.0f));
	cube.transform = transform::rotateY(ctx.getElapsedTime());
	cube.draw();
	gfx.endCustomShader();	
}