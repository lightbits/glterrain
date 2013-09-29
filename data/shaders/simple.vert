#version 140

in vec3 position;
in vec4 color;

out vec4 vertColor;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

void main()
{	
	vertColor = color;
	gl_Position = projection * view * model * vec4(position.x, position.y, position.z, 1.0f);
}