#version 140

in float shadow;

out vec4 outColor;

void main()
{
	vec2 xy = 2.0 * gl_PointCoord.xy - vec2(1.0);
	float r2 = dot(xy, xy);
	outColor.a = exp2(-r2 * 5.0);
	outColor.rgb = vec3(0.98, 0.82, 0.78) * (1.0 - shadow);
	outColor.rgb *= outColor.a; // premultiply alpha for blending
}
