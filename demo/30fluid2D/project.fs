#version 140

in vec2 v_texel;

uniform sampler2D tex_velocity;
uniform sampler2D tex_pressure;
uniform float dx;

out vec4 result;

void main()
{
	vec2 pr = texture(tex_pressure, v_texel + vec2(dx, 0.0)).xy;
	vec2 pl = texture(tex_pressure, v_texel - vec2(dx, 0.0)).xy;
	vec2 pt = texture(tex_pressure, v_texel + vec2(0.0, dx)).xy;
	vec2 pb = texture(tex_pressure, v_texel - vec2(0.0, dx)).xy;
	vec2 grad = vec2(pr.x - pl.x, pt.y - pb.y) * (1.0 / (2.0 * dx));
	vec2 u = texture(tex_velocity, v_texel).xy;
	result.xy = u - grad;
	result.zw = vec2(0.0);
}