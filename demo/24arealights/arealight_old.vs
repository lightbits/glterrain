#version 140

in vec3 position;
in vec3 normal;

out vec3 v_position; // World position
out vec3 v_normal; // World normal

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

void main()
{
	v_position = (model * vec4(position, 1.0)).xyz;
	v_normal = (model * vec4(normal, 0.0)).xyz;
	gl_Position = projection * view * vec4(v_position, 1.0);
}