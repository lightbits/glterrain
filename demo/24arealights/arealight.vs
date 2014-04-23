#version 140

in vec3 position;
in vec3 normal;

out vec3 v_normal;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

void main()
{
	v_normal = (view * model * vec4(normal, 0.0)).xyz;
	gl_Position = projection * view * model * vec4(position, 1.0);
}