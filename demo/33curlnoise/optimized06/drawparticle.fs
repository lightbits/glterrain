#version 430

in float lifetime;
in float distance;
in vec2 shadowTexel;

uniform float particleLifetime;
uniform sampler2D shadowmap;

out vec4 outColor;

void main()
{
	float shadow = texture(shadowmap, shadowTexel).a;
	outColor.rgb = vec3(0.88, 0.78, 0.75) * (1.0 - shadow) + shadow * vec3(0.1, 0.12, 0.18);

	// outColor.rgb *= exp(-distance * 0.8 + 1.5);

	// Alpha blending
	vec2 xy = 2.0 * gl_PointCoord.xy - vec2(1.0);
	float r = xy.x * xy.x + xy.y * xy.y;
	outColor.a = exp(-r * 3.5);

	// Smooth alphablending into and out of existence
	float s = clamp(lifetime / particleLifetime, 0.0, 1.0);
	outColor.a *= s;

	// Premultiply alpha
	outColor.rgb *= outColor.a;
}