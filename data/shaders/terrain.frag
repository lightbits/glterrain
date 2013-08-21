#version 140

in vec4 worldNormal;

uniform float white;

out vec4 outColor;

void main()
{
	// Color surface using the world-transformed normal or pure white
	outColor = mix((worldNormal * 0.5 + vec4(0.5)), vec4(1.0), white);
}