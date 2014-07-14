#version 430

in vec4 mask0;

out vec4 out0;

void main()
{
	vec2 xy = 2.0 * gl_PointCoord.xy - vec2(1.0);
	float r2 = dot(xy, xy);
	float opacity = exp2(-r2 * 5.0) * 0.025;
	out0 = opacity * mask0;
}
