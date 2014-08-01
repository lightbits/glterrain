#version 140

in vec2 v_texel;

uniform sampler2D tex_velocity;
uniform sampler2D tex_temperature;
uniform sampler2D tex_density;
uniform float ambient;
uniform float sigma;
uniform float kappa;
uniform float dt;

out vec4 result;

void main()
{
	float T = texture(tex_temperature, v_texel).r;

	result.rg = texture(tex_velocity, v_texel).rg;

	if (T > ambient)
	{
		float D = texture(tex_density, v_texel).r;
		float f = -kappa * D + sigma * (T - ambient);
		result.g += dt * f;
	}
}