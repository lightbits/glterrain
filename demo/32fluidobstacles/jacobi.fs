#version 140

in vec2 v_texel;

uniform sampler2D tex_input;
uniform sampler2D tex_source;
uniform sampler2D tex_obstacles;

// Iteration parameters
uniform float alpha;
uniform float beta;
uniform float dx; // 1 / grid_size

out vec4 result; // The next iteration

void main()
{
	vec2 xr = texture(tex_input, v_texel + vec2(dx, 0.0)).xy;
	vec2 xl = texture(tex_input, v_texel - vec2(dx, 0.0)).xy;
	vec2 xt = texture(tex_input, v_texel + vec2(0.0, dx)).xy;
	vec2 xb = texture(tex_input, v_texel - vec2(0.0, dx)).xy;
	vec2 xc = texture(tex_input, v_texel).xy;

	// Neighbor obstacle values
	float or = texture(tex_obstacles, v_texel + vec2(dx, 0.0)).r;
	float ol = texture(tex_obstacles, v_texel - vec2(dx, 0.0)).r;
	float ot = texture(tex_obstacles, v_texel + vec2(0.0, dx)).r;
	float ob = texture(tex_obstacles, v_texel - vec2(0.0, dx)).r;

	vec2 bc = texture(tex_source, v_texel).xy;

	// Use center for solid cells
	if (or < 1) xr = xc;
	if (ol < 1) xl = xc;
	if (ot < 1) xt = xc;
	if (ob < 1) xb = xc;

	result.xy = (1.0 / beta) * (xr + xl + xt + xb + alpha * bc);
	result.zw = vec2(0.0);
}