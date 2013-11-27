#version 140

in vec2 position;
in vec2 texel;

out vec2 vertTexel;
out vec2 vertPosition;

void main()
{
	gl_Position = vec4(position, 0.0, 1.0);
	vertTexel = texel;
	vertPosition = position;
}