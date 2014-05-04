#version 140

in vec2 v_texel;

uniform sampler2D velocity;
uniform sampler2D quantity;
uniform float dt;

out vec4 result;

void main()
{
	// Trace velocity field back in time
	vec2 u = texture(velocity, v_texel).xy;
	vec2 texel0 = v_texel - u * dt;
	
	// Sample the quantity to be advected
	result = texture(quantity, texel0);
}