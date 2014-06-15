#version 430
layout (binding = 0, rgba16f) uniform readonly image2D tex;
out vec4 outColor;

void main()
{
	ivec2 texel = ivec2(gl_FragCoord.xy);
	outColor = imageLoad(tex, texel);
}