#version 430

in vec3 vPosition;

out vec4 outColor;

void main()
{
	vec3 rgb = vPosition * 0.5 + vec3(0.5);
	outColor.rgb = rgb;
	outColor.a = 1.0;
}