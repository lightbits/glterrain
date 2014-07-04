#version 430

in float distance;

out vec4 outColor;

void main()
{
	// From drawparticle.fs (rounded edges)
	vec2 xy = 2.0 * gl_PointCoord.xy - vec2(1.0);
	float r = xy.x * xy.x + xy.y * xy.y;
	r *= r;
	outColor.a = exp(-r * 3.5) * 0.05;

	outColor.rgb = vec3(0.0);
}