#version 140

in vec2 position;
in vec2 texel;

out vec2 vertTexel;

void main()
{	
	vertTexel = texel;
	gl_Position = vec4(position, 0.0f, 1.0f);
}