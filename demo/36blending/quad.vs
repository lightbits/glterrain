#version 140

in vec3 position;
in vec2 texel;

out vec2 vTexel;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

void main()
{	
	vTexel = texel;
	gl_Position = projection * view * model * vec4(position, 1.0);
}