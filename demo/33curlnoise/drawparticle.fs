#version 430

in float lifetime;
in float distance;
in vec2 shadowTexel;

uniform float particleLifetime;
uniform sampler2D shadowmap;

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

float sampleShadowMap(vec2 texel)
{
	float s = 0.0f;
	vec3 delta = vec3(1.0 / 256.0, 1.0 / 256.0, 0.0);
	s += texture(shadowmap, texel + delta.xz).a;
	s += texture(shadowmap, texel - delta.xz).a;
	s += texture(shadowmap, texel + delta.zy).a;
	s += texture(shadowmap, texel - delta.zy).a;
	return s * 0.25;
}

void main()
{
	float shadow = sampleShadowMap(shadowTexel);
	outColor.rgb = vec3(0.88, 0.78, 0.75) * (1.0 - shadow) + shadow * vec3(0.1, 0.12, 0.18);

	// outColor.rgb *= exp(-distance * 0.8 + 1.5);

	// Alpha blending
	vec2 xy = 2.0 * gl_PointCoord.xy - vec2(1.0);
	float r = xy.x * xy.x + xy.y * xy.y;
	// r *= r;
	outColor.a = exp(-r * 3.5);

	// Lifetime color grading
	float s = linramp(lifetime / 1.5);
	outColor.a *= s;

	// Required for front-to-back alpha blending
	outColor.rgb *= outColor.a;
}