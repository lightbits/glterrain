#version 140

in vec3 position;
in vec4 color;

out vec4 v_color;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

void main()
{	
	v_color = color;
	gl_Position = projection * view * model * vec4(position, 1.0f);
}