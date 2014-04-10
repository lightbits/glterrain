#version 140

in vec2 v_texel;
in vec3 v_world_pos;
out vec4 out_color;

uniform sampler2D tex_normal;
uniform sampler2D tex_diffuse;

vec4 getLightContribution(vec3 light_pos, vec4 light_col, vec4 diffuse, vec3 p, vec3 n)
{
	vec3 dir = normalize(light_pos - p);
	float intensity = dot(dir, n);
	intensity = max(intensity, 0.0);
	return intensity * light_col * diffuse;
}

void main()
{
	vec4 diffuse = texture(tex_diffuse, v_texel);
	vec3 normal = texture(tex_normal, v_texel).xyz * 2.0 - vec3(1.0);

	vec4 color = vec4(0.0);
	color += getLightContribution(
		vec3(0.0, 0.8, 0.3), 
		vec4(1.0, 0.8, 0.5, 1.0),
		diffuse,
		v_world_pos,
		normal);

	color += getLightContribution(
		vec3(1.0, 0.8, 0.3), 
		vec4(1.4, 1.2, 1.1, 1.0),
		diffuse,
		v_world_pos,
		normal);

	color += getLightContribution(
		vec3(-1.0, 0.8, -0.3), 
		vec4(1.4, 1.2, 1.1, 1.0),
		diffuse,
		v_world_pos,
		normal);

	color += getLightContribution(
		vec3(0.0, 2.5, -0.8), 
		vec4(1.4, 1.2, 1.1, 1.0),
		diffuse,
		v_world_pos,
		normal);

	out_color = color;
}