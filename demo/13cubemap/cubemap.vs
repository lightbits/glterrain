#version 150

in vec3 position;
in vec4 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 v_color;

void main()
{
	v_color = color;
	gl_Position = projection * view * model * vec4(position, 1.0);
}