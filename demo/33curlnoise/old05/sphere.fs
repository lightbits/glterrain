#version 430

in float vDistToCamera;
in vec3 vNormal;

uniform float sinkSourceBlend;

out vec4 outColor;

void main()
{
	vec3 N = vNormal * 0.5 + vec3(0.5);
	outColor.rgb = N * 0.5 + vec3(0.5);
	outColor.a = 0.7;

	vec3 sinkColor = vec3(1.0, 0.55, 0.4);
	vec3 sourceColor = vec3(0.4, 0.55, 1.0);
	vec3 color = mix(sinkColor, sourceColor, sinkSourceBlend);
	outColor.rgb *= color;

	float fogAmount = exp(-vDistToCamera * 0.3 + 1.0);
	outColor.rgb *= fogAmount;
}