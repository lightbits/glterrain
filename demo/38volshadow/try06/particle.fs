#version 430

in vec3 color;

out vec4 outColor;

void main()
{
	vec2 xy = 2.0 * gl_PointCoord.xy - vec2(1.0);
	float r2 = dot(xy, xy);
	outColor.a = exp2(-r2 * 5.0);
	outColor.rgb = color * outColor.a;
}
