#version 140

in vec4 worldNormal;
in vec4 worldPos;
in float distToCamera;
in float height;

uniform float white;

out vec4 outColor;

void main()
{
	// https://lva.cg.tuwien.ac.at/ecg/wiki/doku.php?id=students:gooch

  	// Diffuse lighting
	// vec4 ambient = vec4(0.2, 0.23, 0.35, 1.0);
	vec4 lightColor = vec4(1.0, 0.8, 0.5, 1.0);
	vec4 ambient = vec4(67.0/255.0, 66.0/255.0, 63.0/255.0, 1.0);
	// vec4 lightColor = vec4(135.0/255.0, 105.0/255.0, 100.0/255.0, 1.0);
	vec4 dirToLight = -normalize(vec4(1.0, -1.0, 0.0, 0.0));
	float intensity = max(0, dot(dirToLight, worldNormal));
	outColor = intensity * lightColor + (1.0 - intensity) * ambient;
	outColor = mix(outColor, vec4(1.0), white);

  	// Fog
	const float LOG2 = 1.442695;
	const float density = 0.05125;
	vec4 fogColor = vec4(0.77, 0.68, 0.68, 1.0);
	float z = distToCamera;
	float fogFactor = 1.0 - clamp(exp2(-density * density * z * z * LOG2), 0.0, 1.0);
	outColor = mix(outColor, fogColor, fogFactor);

	// Color surface using the world-transformed normal or pure white
	// outColor = mix((worldNormal * 0.5 + vec4(0.5)), vec4(1.0), white);
}