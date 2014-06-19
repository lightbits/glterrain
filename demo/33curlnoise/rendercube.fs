#version 430

in vec3 vColor;
in vec3 vPosition;
out vec4 outColor;

void main()
{
	outColor.rgb = vec3(0.9, 0.55, 0.43);
	outColor.rgb *= 2.5 - (vPosition.z * 0.5 + 0.5);
	outColor.a = 1.0;
}