#version 140

in vec2 v_texel;

uniform sampler2D tex_velocity;
uniform sampler2D tex_obstacles;
uniform float dx;

out vec4 result;

void main()
{
	float solid = texture(tex_obstacles, v_texel).r;
	if (solid < 1)
	{
		result = vec4(0.0);
		return;
	}

	vec2 ur = texture(tex_velocity, v_texel + vec2(dx, 0.0)).xy;
	vec2 ul = texture(tex_velocity, v_texel - vec2(dx, 0.0)).xy;
	vec2 ut = texture(tex_velocity, v_texel + vec2(0.0, dx)).xy;
	vec2 ub = texture(tex_velocity, v_texel - vec2(0.0, dx)).xy;

	float or = texture(tex_obstacles, v_texel + vec2(dx, 0.0)).r;
	float ol = texture(tex_obstacles, v_texel - vec2(dx, 0.0)).r;
	float ot = texture(tex_obstacles, v_texel + vec2(0.0, dx)).r;
	float ob = texture(tex_obstacles, v_texel - vec2(0.0, dx)).r;

	// Use obstacle velocity at borders
	if (or < 1) ur *= 0.0;
	if (ol < 1) ul *= 0.0;
	if (ot < 1) ut *= 0.0;
	if (ob < 1) ub *= 0.0;

	result.xy = vec2(ur.x - ul.x + ut.y - ub.y) * (1.0 / (2.0 * dx));
	result.zw = vec2(0.0);
}