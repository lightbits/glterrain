#version 140

in vec2 position;

out vec2 vertUV;

void main()
{	
	vertUV = position;
	gl_Position = vec4(position, 0.2f, 1.0f);
}