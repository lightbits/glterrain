#version 140

in vec2 v_texel;

uniform sampler2D field;

out vec4 result;

void main()
{
	float dx = v_texel.x - 0.5;
	float dy = v_texel.y - 0.5;
	result.xy = vec2(0.0, 1.0) * exp(-0.1 * (dx * dx + dy * dy));
	result.zw = vec2(0.0);
}