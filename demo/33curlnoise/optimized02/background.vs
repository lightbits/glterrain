#version 430

in vec3 position;

out vec3 dir;

uniform mat4 view;

void main()
{
	dir = (view * vec4(position.xy, -2.0, 1.0)).xyz;
	gl_Position = vec4(position, 1.0);
}