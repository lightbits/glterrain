#version 430

in vec4 position;

uniform mat4 projection;
uniform mat4 view;

void main()
{
	gl_Position = projection * view * position;
	gl_PointSize = 20.0;
}
