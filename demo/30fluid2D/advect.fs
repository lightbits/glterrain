#version 140

in vec2 v_texel;

uniform sampler2D tex_velocity;
uniform sampler2D tex_source;
uniform float dt;

out vec4 result;

void main()
{
	// Trace velocity field back in time
	vec2 u = texture(tex_velocity, v_texel).xy;
	vec2 texel0 = v_texel - u * dt;
	
	// Sample the quantity to be advected
	result = texture(tex_source, texel0);
}