#version 430
in vec3 position;
in vec3 color;

out vec3 vColor;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
	vColor = color;
	gl_Position = projection * view * model * vec4(position, 1.0);
}