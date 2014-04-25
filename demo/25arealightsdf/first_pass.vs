#version 140

in vec3 position;
in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 v_view_position;
out vec3 v_view_normal;

void main()
{
	v_view_position = (view * model * vec4(position, 1.0)).xyz;
	v_view_normal   = (view * model * vec4(normal, 0.0)).xyz;
	gl_Position     = projection * vec4(v_view_position, 1.0);
}