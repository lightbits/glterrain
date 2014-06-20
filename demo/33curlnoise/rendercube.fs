#version 430

in float lifetime;
in vec3 vPosition;
out vec4 outColor;

void main()
{
	outColor.rgb = vec3(0.9, 0.45, 0.33);
	outColor.a = 1.0;

	// Fog-ish thing
	outColor.rgb *= 2.5 - (vPosition.z * 0.5 + 0.5);

	// Speed color grading
	// outColor.rgb *= lifetime / 10.0;

	// Alpha blending
	// vec2 xy = 2.0 * gl_PointCoord.xy - vec2(1.0);
	// float r = xy.x * xy.x + xy.y * xy.y;
	// outColor.a = exp(-r * 1.5);
}