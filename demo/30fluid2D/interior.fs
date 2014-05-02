#version 140

in vec2 v_texel;

uniform sampler2D tex_velocity;
uniform sampler2D tex_pressure;

out vec3 out_velocity;
out vec3 out_pressure;

void main()
{
	vec3 v = texture(tex_velocity, v_texel).rgb;
	vec3 p = texture(tex_pressure, v_texel).rgb;

	out_velocity = 1.2 * vec3(v_texel, 0.0) + 0.5 * v;
	out_pressure = p;
}