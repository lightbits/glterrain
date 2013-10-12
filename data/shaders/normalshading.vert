#version 140

in vec3 position;
in vec3 normal;

out vec4 vertColor;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

void main()
{	
	vertColor = vec4(normal * 0.5 + vec3(0.5), 1.0);
	gl_Position = projection * view * model * vec4(position, 1.0f);
}