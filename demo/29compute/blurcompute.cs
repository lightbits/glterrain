#version 430

layout (local_size_x = 3, local_size_y = 3) in;
layout (binding = 0, rgba16f) uniform readonly image2D inTex;
layout (binding = 1, rgba16f) uniform image2D outTex;

shared vec4 sum;

void main()
{
	ivec2 texel = ivec2(gl_WorkGroupID.xy);
	ivec2 sample_texel = texel + ivec2(gl_LocalInvocationID.xy) - ivec2(1, 1);

	vec4 color = imageLoad(inTex, sample_texel);
	sum = color;
	imageStore(outTex, texel, sum);
}