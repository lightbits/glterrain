#version 140

in vec3 position;
in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 world_normal; // world-space normal direction
out vec3 view_normal; // view-space normal direction
out vec4 world_pos; // world-space position
out vec3 dir_to_viewer; // normalized vector pointing towards the viewer

void main()
{
	world_pos = model * vec4(position, 1.0);
	vec4 view_pos = view * world_pos;
	gl_Position = projection * view_pos;

	// assuming camera centered at (0, 0, 0)
	dir_to_viewer = normalize(-view_pos.xyz); // normalizing here might cause distortions

	world_normal = normalize(model * vec4(normal, 0.0)).xyz;
	view_normal = normalize(view * vec4(world_normal, 0.0)).xyz;
}