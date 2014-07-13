#version 430

in float sampleSum;

out vec4 outColor;

void main()
{
	outColor = vec4(sampleSum);
}