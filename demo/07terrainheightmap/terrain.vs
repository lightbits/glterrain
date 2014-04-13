#version 140

in vec3 position;
in vec2 texel;

out vec2 v_texel;
out vec3 v_world_pos;
out vec3 v_dir_to_viewer;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform sampler2D tex_height;
uniform float terrain_height;

void main()
{
	v_texel = texel;
	float h = texture(tex_height, texel).x * terrain_height;

	vec4 world_pos = model * vec4(position.x, position.y + h, position.z, 1.0);
	vec4 view_pos = view * world_pos;
	v_world_pos = world_pos.xyz;
	v_dir_to_viewer = normalize(-view_pos.xyz);
	gl_Position = projection * view_pos;
}