#version 140

in vec2 v_texel;

uniform sampler2D tex;

out vec4 out_color;

vec3 mapToColor(float t)
{
	vec3 c0 = vec3(0.0, 0.0, 1.0); // 0
	vec3 c1 = vec3(0.0, 1.0, 1.0); // 0.33
	vec3 c2 = vec3(1.0, 1.0, 0.0); // 0.66
	vec3 c3 = vec3(1.0, 0.0, 0.0); // 1.0

	if (t <= 0.33)
		return mix(c0, c1, t);
	if (t <= 0.66)
		return mix(c1, c2, t);
	if (t <= 1.0)
		return mix(c2, c3, t);
}

void main()
{
	vec2 f = texture(tex, v_texel).xy;

	out_color = vec4(abs(f), 0.0, 1.0);
}