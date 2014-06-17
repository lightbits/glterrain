#version 430
layout (binding = 0, rgba32f) uniform readonly image2D tex;
out vec4 outColor;

void main()
{
	ivec2 texel = ivec2(gl_FragCoord.xy);

	int m = 2;

	vec4 v00 = imageLoad(tex, texel + ivec2(-m - 1, -m - 1));
	vec4 v10 = imageLoad(tex, texel + ivec2(m, -m - 1));
	vec4 v01 = imageLoad(tex, texel + ivec2(-m - 1, m));
	vec4 v11 = imageLoad(tex, texel + ivec2(m, m));

	// Calculate sum in rectangle
	vec4 sum = v11 - v01 - v10 + v00;

	// Divide by area to get average
	sum /= float((m * 2 + 1) * (m * 2 + 1));

	outColor = sum;
}