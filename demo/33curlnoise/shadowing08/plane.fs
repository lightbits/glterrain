#version 140

in vec3 vPosition;
in vec3 vNormal;
in float dist;

out vec4 outColor;

uniform vec3 color;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 ambientColor;
uniform mat4 projectionLight;
uniform mat4 viewLight;
uniform sampler2D shadowmap;

vec3 lighting(vec3 N, vec3 D, vec3 Ld, vec3 Lp, vec3 P)
{
	vec3 diff = Lp - P;
	float len = length(diff);
	vec3 L = diff / len;
	float NdotL = max(dot(N, L), 0.0);
	float Id = NdotL / (len * len);
	return Id * Ld * D;
}

vec3 sun(vec3 N, vec3 D, vec3 A, vec3 Ld, vec3 L)
{
	float NdotL = max(dot(N, L), 0.0);
	return NdotL * D * Ld + (1.0 - NdotL) * A;
}

void main()
{
	vec3 N = normalize(vNormal);
	outColor.rgb = lighting(N, color, lightColor, lightPos, vPosition);
	outColor.rgb += sun(N, color, ambientColor, vec3(0.9, 0.92, 1.0) * 0.4, normalize(vec3(0.5, 1.0, 1.0)));
	outColor.a = 1.0;

	outColor.rgb *= 0.00001;
	outColor.rgb += pow(color, vec3(2.2));

	// shadow from particles
	vec4 posFromLight = projectionLight * viewLight * vec4(vPosition, 1.0);
	vec2 shadowTexel = posFromLight.xy * 0.5 + vec2(0.5);
	float shadow = texture(shadowmap, shadowTexel).a;
	outColor.rgb *= (1.0 - shadow);

	outColor.rgb = mix(outColor.rgb, vec3(0.0), dist / 13.0);

	// gamma correction
	outColor.rgb = pow(outColor.rgb, vec3(1.0 / 2.2));
}