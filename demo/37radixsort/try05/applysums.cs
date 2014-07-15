#version 430

layout (local_size_x = 4) in; // Must equal <block_size> in app.cpp
layout (std430, binding = 0) buffer ScanBuffer {
	uvec4 Scan[];
};

layout (std430, binding = 1) buffer SumsBuffer {
	uvec4 Sums[];
};

layout (std430, binding = 2) buffer KeyBuffer {
	uint Key[];
};

shared uvec4 sum;
shared uvec4 last;
uniform int bitOffset;

void main()
{
    const uint radix = 4;
    sum = Sums[gl_WorkGroupID.x];
    barrier();
    //Scan[gl_GlobalInvocationID.x] += sum;
    //barrier();

    // Add the last value of the previous scan to each scan (except the first)
    uint last_key = (Key[gl_NumWorkGroups.x * gl_WorkGroupSize.x - 1] >> bitOffset) & (radix - 1);
    uvec4 temp = Scan[gl_NumWorkGroups.x * gl_WorkGroupSize.x - 1] + Sums[gl_NumWorkGroups.x - 1];

    // Get inclusive sum
    temp += uvec4(last_key == 0 ? 1 : 0, last_key == 1 ? 1 : 0, last_key == 2 ? 1 : 0, last_key == 3 ? 1 : 0);
    
    last.x = 0;
    last.y = temp.x;
    last.z = temp.x + temp.y;
    last.w = temp.x + temp.y + temp.z;

    Scan[gl_GlobalInvocationID.x] += sum + last;
}