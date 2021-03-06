#version 140

in float visibility;

uniform vec3 color;

out vec4 out_color;

void main() 
{
	out_color = vec4(color, 0.0) * visibility;

	// Gamma correction
	// out_color.rgb = pow(out_color.rgb, vec3(1.0 / 2.2));
}