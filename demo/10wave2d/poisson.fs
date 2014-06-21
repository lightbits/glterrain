#version 140

in vec2 v_texel;

uniform sampler2D tex_data;
uniform float dt;
uniform float dx;
uniform float dissipation;
uniform float wave_c;
uniform vec2 mpos;

out vec4 out_data;

void main()
{	
	float vc = texture(tex_data,   v_texel).g;
	float ur = texture(tex_data,   v_texel + vec2(dx, 0.0)).r;
	float ul = texture(tex_data,   v_texel - vec2(dx, 0.0)).r;
	float ut = texture(tex_data,   v_texel + vec2(0.0, dx)).r;
	float ub = texture(tex_data,   v_texel - vec2(0.0, dx)).r;
	float uc = texture(tex_data,   v_texel).r;
	out_data.g = dissipation * vc;
	out_data.g += (dt * wave_c) * (ur + ul + ut + ub - 4.0 * uc) / (dx * dx);
	out_data.r = uc + dt * out_data.g;

	// Forces from mouse
	if (length(v_texel - mpos) < 0.025) 
	{
		// out_data.g = 0.1;
		out_data.r = min(out_data.r + 0.5 * dt, 0.3);
	}
	
	out_data.ba = vec2(0.0);
}