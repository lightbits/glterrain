#version 430

in float lifetime;
in float distance;

uniform float particleLifetime;

out vec4 outColor;

float linramp(float r)
{
    if (r >= 1.0)
        return 1.0;
	else if (r < 0.0)
		return 0.0;
	else
		return r;
}

void main()
{
	outColor.rgb = vec3(0.88, 0.78, 0.75);
	outColor.a = 1.0;

	// Fog based on distance into scene
	float fogAmount = exp(-distance * 0.8 + 2.0);
	vec3 fogColor = vec3(0.15, 0.17, 0.25);
	outColor.rgb = mix(fogColor, outColor.rgb, fogAmount);

	// Alpha blending
	vec2 xy = 2.0 * gl_PointCoord.xy - vec2(1.0);
	float r = xy.x * xy.x + xy.y * xy.y;
	outColor.a = exp(-r * 3.5);

	// Lifetime color grading
	float s = linramp(lifetime / particleLifetime);
	outColor.a *= s;

	// Fire!
	outColor.rgb = mix(outColor.rgb, vec3(1.0, 0.4, 0.1), pow(s, 5.0));
}