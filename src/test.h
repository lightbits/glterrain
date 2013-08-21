struct StaticMesh
{
	Texture *texture;
	Program *program;
	GLuint vbo;
	GLuint vao;
};

struct DynamicMesh
{
	Texture *texture;
	Program *program;
	std::vector<vec3> vertices;
	std::vector<vec3> normals;
	std::vector<vec4> colors;
	std::vector<vec2> texels;
};

struct StaticMeshInstance
{
	StaticMesh *mesh;
	mat4 modelTransform;
};