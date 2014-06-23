#version 430

in vec3 position;
in vec2 texel;

out vec2 vTexel;

void main()
{
	vTexel = texel;
	gl_Position = vec4(position, 1.0);
}