#version 140

in vec2 v_texel;

uniform sampler2D tex_input;
uniform vec2 impulse_pos;
uniform float impulse_radius;
uniform float impulse_effect;

out vec4 result;

void main()
{
	float len = length(v_texel - vec2(impulse_pos.x, impulse_pos.y));
	float f = max(impulse_radius * 1.0 - len, 0.0);

	vec4 u = texture(tex_input, v_texel);
	result = (1.0 - f) * u + f * vec4(impulse_effect);
}