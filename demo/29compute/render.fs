#version 430

in float depth;
in vec3 vNormal;
in vec3 color;

out vec4 outColor;

void main()
{
	vec3 N = vNormal * 0.5 + vec3(0.5);
	outColor.rgb = N * 0.5 + vec3(0.5);
	outColor.rgb *= color;
	// outColor.rgb *= exp(-depth * depth * 0.16);
	outColor.a = 1.0;
}