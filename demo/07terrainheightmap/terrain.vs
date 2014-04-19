#version 140

in vec3 position;
in vec2 texel;

out vec2 v_texel;
out vec3 v_view_pos;
out vec3 v_view_dir;
out vec3 T;
out vec3 B;
out vec3 U;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform sampler2D tex_height;
uniform float terrain_height;
uniform int terrain_res_x;
uniform int terrain_res_y;

void main()
{
	v_texel = texel;

	// Calculate tangent frame
	float dx = 1.0 / terrain_res_x;
	float dy = 1.0 / terrain_res_y;
	float h00 = texture(tex_height, texel).x * terrain_height;
	float h10 = texture(tex_height, texel + vec2(dx, 0.0)).x * terrain_height;
	float h01 = texture(tex_height, texel + vec2(0.0, dy)).x * terrain_height;
	T = normalize(vec3(dx, h10 - h00, 0.0));
	B = normalize(vec3(0.0, h01 - h00, dy));
	U = cross(B, T);

	// Displace the vertex by the terrain height
	v_view_pos = (view * model * vec4(position.x, position.y + h00, position.z, 1.0)).xyz;
	v_view_dir = normalize(-v_view_pos);
	gl_Position = projection * vec4(v_view_pos, 1.0);
}