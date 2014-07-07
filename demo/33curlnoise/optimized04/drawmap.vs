#version 430

in vec2 texel;
in vec2 position;

out vec2 vTexel;

void main()
{
	vTexel = texel;
	gl_Position = vec4(position, 0.0, 1.0);
}