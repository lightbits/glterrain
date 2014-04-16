
struct Material
{
	ShaderProgram *shader;

	vec4     color;
	Texture *diffuse_map;
	Texture *normal_map;
};

void Renderer::useMaterial(Material &material) { currentMaterial = &material; }