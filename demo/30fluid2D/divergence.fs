#version 140

in vec2 v_texel;

uniform sampler2D tex_velocity;
uniform float dx;

out vec4 result;

void main()
{
	vec2 ur = texture(tex_velocity, v_texel + vec2(dx, 0.0)).xy;
	vec2 ul = texture(tex_velocity, v_texel - vec2(dx, 0.0)).xy;
	vec2 ut = texture(tex_velocity, v_texel + vec2(0.0, dx)).xy;
	vec2 ub = texture(tex_velocity, v_texel - vec2(0.0, dx)).xy;
	result.xy = vec2(ur.x - ul.x + ut.y - ub.y) * (1.0 / (2.0 * dx));
	result.zw = vec2(0.0);
}