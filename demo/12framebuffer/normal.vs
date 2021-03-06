#version 140

in vec3 position;
in vec3 normal;

out vec3 world_normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0);
	world_normal = normalize((model * vec4(normal, 0.0)).xyz);
}