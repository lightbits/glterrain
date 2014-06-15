#version 140

in vec3 normal;
in vec3 position;

out float visibility; // 1 if front is facing towards camera

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() 
{
	vec4 view_pos = view * model * vec4(position, 1.0);
	vec4 view_normal = view * model * vec4(normal, 0.0);

	vec3 V = normalize(-view_pos.xyz); // Direction to viewer

	if (dot(V, view_normal.xyz) > 0.0) {
		visibility = 1.0;
	}

	gl_Position = projection * view_pos;
}