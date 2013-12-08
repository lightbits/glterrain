#version 140

in vec2 vert_position;
in vec2 vert_texel;

uniform vec3 light0_pos;
uniform vec3 light0_col;
uniform vec3 ambient;
uniform float time;

uniform sampler2D refraction_tex;
uniform sampler2D reflection_tex;
uniform sampler2D water_normals_tex;
uniform mat4 view;
out vec4 out_color;

void main()
{
	vec4 refraction = texture(refraction_tex, vert_texel);
	float terrain_y = refraction.w;

	// Sample wave normal from texture
	float sample_freq = 0.5;
	float damping = terrain_y;
	vec4 ripple_normal = texture(water_normals_tex, vert_texel * sample_freq + time * 0.1);

	// Use wave normal to distort texel lookup
	vec2 texel = vert_texel;
	texel += ripple_normal.xy * 0.015;
	texel += vec2(sin(time * 3.0 + vert_position.y * 5.0), 0.0) * 0.012;

	// Sample refraction texture
	vec4 c1 = texture(refraction_tex, texel + vec2(0.0, +1.0 / 480.0));
	vec4 c2 = texture(refraction_tex, texel + vec2(0.0, -1.0 / 480.0));
	vec4 c3 = texture(refraction_tex, texel + vec2(+1.0 / 640.0, 0.0));
	vec4 c4 = texture(refraction_tex, texel + vec2(-1.0 / 640.0, 0.0));
	out_color = (c1 + c2 + c3 + c4) / 4.0;

	out_color = mix(out_color, texture(reflection_tex, vert_texel), 0.5);

	// out_color = vec4(terrain_y);
	// if (terrain_y < 0.05)
	// 	out_color = mix(texture(refraction_tex, vert_texel), texture(reflection_tex, vert_texel), 0.5);
	// else
	// 	out_color = texture(reflection_tex, vert_texel);

	// vec3 light_dir = normalize(vec3(1.0, 0.0, 5.0));
	// float n_dot_l = max(0, dot(light_dir, ripple_normal.xyz));
	// out_color *= vec4(n_dot_l * light0_col + (1.0 - n_dot_l) * ambient, 1.0);

	// out_color = vec4(ripple_normal.xyz * 0.5 + 0.5, 1.0);
}