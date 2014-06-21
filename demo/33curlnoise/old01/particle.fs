#version 430

in float lifetime;
in float distance;

out vec4 outColor;

void main()
{
	outColor.rgb = vec3(0.9, 0.45, 0.33);
	outColor.a = 1.0;

	// Fog based on distance into scene
	float fogAmount = exp(-distance * 0.4 + 1.0);
	vec3 fogColor = vec3(0.15, 0.17, 0.25);
	outColor.rgb = mix(fogColor, outColor.rgb, fogAmount);

	// Speed color grading
	// outColor.rgb *= lifetime / 10.0;

	// Alpha blending
	// vec2 xy = 2.0 * gl_PointCoord.xy - vec2(1.0);
	// float r = xy.x * xy.x + xy.y * xy.y;
	// outColor.a = exp(-r * 1.5);
}