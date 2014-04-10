#version 140

in vec3 position;
in vec2 texel;

out vec2 v_texel;
out vec3 v_world_pos;

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
	v_world_pos = world_pos.xyz;
	gl_Position = projection * view * world_pos;
}