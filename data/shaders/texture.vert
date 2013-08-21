#version 140

in vec3 position;
in vec2 texel;

out vec2 vertTexel;

void main()
{
	gl_Position = vec4(position, 1.0);
	vertTexel = texel;
}