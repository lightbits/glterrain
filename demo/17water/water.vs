#version 140

in vec2 position;
in vec2 texel;

out vec2 vert_texel;
out vec2 vert_position;

void main()
{
	gl_Position = vec4(position, 0.0, 1.0);
	vert_texel = texel;
	vert_position = position;
}