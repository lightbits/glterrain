#version 140

in vec2 texel;
in vec3 position;

out vec3 v_normal;

uniform sampler2D heightmap;

uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;

void main()
{	
	float h = texture(heightmap, texel).r;
	vec3 p = vec3(position.x, h, position.z);

	// TODO: Flat shading, compute normal

	gl_Position = projection * view * model * vec4(p, 1.0);
}