#version 140

in vec2 v_texel;

uniform sampler2D tex_input;
uniform sampler2D tex_source;

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
	vec2 bc = texture(tex_source, v_texel).xy;
	result.xy = (1.0 / beta) * (xr + xl + xt + xb + alpha * bc);
	result.zw = vec2(0.0);
}