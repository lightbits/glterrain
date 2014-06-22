#version 430

in vec3 position;
in vec2 texel;

out vec3 vTexel;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0);
	vTexel.xy = texel;
	vTexel.z = position.z * 0.5 + 0.5;
}