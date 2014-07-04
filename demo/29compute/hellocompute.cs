#version 430

layout (local_size_x = 16, local_size_y = 16) in;
layout (binding = 0, rgba16f) uniform writeonly image2D tex;

// As specified in the code in app.cpp
const int TEXTURE_SIZE_X = 128;
const int TEXTURE_SIZE_Y = 128;

void main()
{
	// The integer coordinate texture coordinate (from 0 to TEXTURE_SIZE_X/Y - 1)
	ivec2 texel = ivec2(gl_GlobalInvocationID.xy);

	// Results in a 128x128 image with a smooth varying
	// color gradient of black in the lower-left and yellow
	// in the top-right
    //float u = float(gl_GlobalInvocationID.x) / TEXTURE_SIZE_X;
    //float v = float(gl_GlobalInvocationID.y) / TEXTURE_SIZE_Y;
    //imageStore(tex, texel, vec4(u, v, 0.0, 1.0));

	// A similiar image where 16x16 blocks of pixels have the same color
    //float u = float(gl_WorkGroupID.x) / gl_NumWorkGroups.x;
    //float v = float(gl_WorkGroupID.y) / gl_NumWorkGroups.y;
    //imageStore(tex, texel, vec4(u, v, 0.0, 1.0));

	// A 128x128 image where repeated blocks of 16x16 pixels
	// have the same gradient as the first image
    float u = float(gl_LocalInvocationID.x) / gl_WorkGroupSize.x;
    float v = float(gl_LocalInvocationID.y) / gl_WorkGroupSize.y;
    imageStore(tex, texel, vec4(u, v, 0.0, 1.0));
}