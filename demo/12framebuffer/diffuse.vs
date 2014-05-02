#version 140

in vec3 position;
in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 view_normal; // view-space normal direction
out vec3 view_position; // view-space position
out vec3 dir_to_viewer; // normalized vector pointing towards the viewer

void main()
{
	vec3 view_position = (view * model * vec4(position, 1.0)).xyz;
	gl_Position = projection * vec4(view_position, 1.0);
	dir_to_viewer = normalize(-view_position.xyz); // normalizing here might cause distortions

	view_normal = normalize(view * model * vec4(normal, 0.0)).xyz;
}