#version 140

in vec2 v_texel;

uniform sampler2D tex_velocity;
uniform sampler2D tex_source;
uniform sampler2D tex_obstacles;
uniform float dissipation;
uniform float dt;

out vec4 result;

void main()
{
	float solid = texture(tex_obstacles, v_texel).r;
	if (solid < 0.5)
	{
		result = vec4(0.0);
		return;
	}

	// Trace velocity field back in time
	vec2 u = texture(tex_velocity, v_texel).xy;
	vec2 texel0 = v_texel - u * dt;
	
	// Sample the quantity to be advected
	result = dissipation * texture(tex_source, texel0);
}