/*
This shader takes as input
	* A heightmap
	* A normalmap
	* Two line vertices (0, 0, 0) and (0, 1, 0), with texel coordinates
and renders the perturbed normals as lines.

First, a tangent frame is calculated from the terrain gradient at this location,
then the perturbed normal is calculated from sampling the normalmap, and blending
with the tangent frame.
*/

#version 140

in vec3 position;
in vec2 texel;
 
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform sampler2D tex_normal;
uniform sampler2D tex_height;
uniform float terrain_height;
uniform int terrain_res_x;
uniform int terrain_res_y;

const float normal_height = 0.05;

void main()
{
	vec3 sample_n = texture(tex_normal, texel).xyz * 2.0 - vec3(1.0);

	// Calculate tangent frame
	float dx = 1.0 / terrain_res_x;
	float dy = 1.0 / terrain_res_y;
	float h00 = texture(tex_height, texel).x * terrain_height;
	float h10 = texture(tex_height, texel + vec2(dx, 0.0)).x * terrain_height;
	float h01 = texture(tex_height, texel + vec2(0.0, dy)).x * terrain_height;

	vec3 T = normalize(vec3(dx, h10 - h00, 0.0));
	vec3 B = normalize(vec3(0.0, h01 - h00, dy));
	vec3 U = cross(B, T);

	// Calculate the final normal (world-space)
	vec3 N = sample_n.x * T + sample_n.y * B + sample_n.z * U;
	N = normalize(N);

	vec4 Pv = view * model * vec4(vec3(position.x, h00, position.z) + N * position.y * normal_height, 1.0);
	gl_Position = projection * Pv;
}