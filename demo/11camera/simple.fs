#version 140

in vec3 vNormal;

out vec4 outColor;

void main()
{
	outColor.rgb = normalize(vNormal) * 0.5 + vec3(0.5);
	outColor.a = 1.0;
}