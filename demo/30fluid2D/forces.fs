#version 140

in vec2 v_texel;

uniform sampler2D tex_velocity;
uniform float dt;
uniform float time;

out vec4 result;

void main()
{
	float dx = v_texel.x - 0.5;
	float dy = v_texel.y - 0.5;

	vec2 u = texture(tex_velocity, v_texel).xy;
	vec2 f = vec2(cos(time), sin(time) + 1.0) * exp(-30.0 * (dx * dx + dy * dy));

	result.xy = u + f * dt;
	result.zw = vec2(0.0);
}