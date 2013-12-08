#version 140

in vec3 position;
in vec4 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 vert_color;
out vec4 world_pos;

void main() 
{
	vert_color = color;
	world_pos = model * vec4(position, 1.0);
	gl_Position = projection * view * world_pos;
}