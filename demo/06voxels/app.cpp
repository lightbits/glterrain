#include "app.h"

VertexArray vao;
ShaderProgram shader;
MeshBuffer voxelbuffer;

mat4 view;

bool load()
{
	if (!shader.loadAndLinkFromFile("./demo/06voxels/simple"))
		return false;

	return true;
}

void free()
{
	voxelbuffer.dispose();
	vao.dispose();
}

Mesh voxelCube(vec3 translation, float scale)
{
	Mesh cube = Mesh::genUnitCube(false, true, true);
	int count = cube.getPositionCount();
	for (int i = 0; i < count; ++i)
	{
		cube.getPosition(i) *= scale;
		cube.getPosition(i) += translation;
	}
		
	return cube;
}

void init(Renderer &gfx, Context &ctx)
{	
	vao.create();
	vao.bind();

	// Create voxel mesh
	Mesh voxelmesh;

	float dp = PI * 2.0 / 16.0;
	float dt = PI / 16.0;
	for (int i = 0; i <= 16; ++i)
	{
		for (int j = 0; j <= 16; ++j)
		{
			float p = i * dp;
			float t = j * dt;
			float y = cos(t);
			float x = sin(t) * cos(p);
			float z = sin(t) * sin(p);
			voxelmesh.addMesh(voxelCube(vec3(x, y, z), 1.0 / 2.0));
		}
	}

	voxelbuffer = MeshBuffer(voxelmesh);
}

void update(Renderer &gfx, Context &ctx, double dt)
{
	view = 
		transform::translate(0.0f, 0.0f, -5.5f) *
		transform::rotateX(-0.23f) * 
		transform::rotateY(ctx.getElapsedTime());
}

void render(Renderer &gfx, Context &ctx, double dt)
{
	gfx.setClearDepth(1.0);
	gfx.setClearColor(0.0f, 0.0f, 0.0f);
	gfx.clearColorAndDepth();
	gfx.setDepthTestState(DepthTestStates::LessThanOrEqual);
	gfx.setCullState(CullStates::CullNone);

	gfx.beginCustomShader(shader);
	gfx.setUniform("view", view);
	gfx.setUniform("projection", glm::perspective(45.0f, ctx.getWidth() / (float)ctx.getHeight(), 0.05f, 10.0f));
	gfx.setUniform("model", mat4(1.0f));
	voxelbuffer.draw();
	gfx.endCustomShader();	
}