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
shared uint lastKey;
uniform int bitOffset;

void main()
{
    const uint radix = 4;

    // This sum value needs to be added to each element in each scan block
    // It is a vec4 because we keep track of 4 scan arrays (one for each radix digit)
    sum = Sums[gl_WorkGroupID.x];
    Scan[gl_GlobalInvocationID.x] += sum; // does not quite work...?
    barrier();

    // Ah! This doesn't work
    // We need the sum to have been added to the last element in the scan array
    // but the barrier() above only causes synchronization within a block!
    // So we can't be sure that the last element of the last block has been updated yet

    // That is, we need to fetch the last sum manually

    // We also need to add the sum from one scan array to the next digit's scan array
    // So we fetch the last scan value from each digit's array
    uint last_i = gl_NumWorkGroups.x * gl_WorkGroupSize.x - 1;
    uvec4 lastScan = Scan[last_i];

    // We need the inclusive sum, so we also fetch the last key and check if it matches
    // the digit. If it does, add one to that array.
    lastKey = (Key[last_i] >> bitOffset) & (radix - 1);
    lastScan += uvec4(lastKey == 0 ? 1 : 0, lastKey == 1 ? 1 : 0, lastKey == 2 ? 1 : 0, lastKey == 3 ? 1 : 0);
    
    // Finally, add up the values to add to the scan arrays
    uvec4 prefix = uvec4(0, lastScan.x, lastScan.x + lastScan.y, lastScan.x + lastScan.y + lastScan.z);

    Scan[gl_GlobalInvocationID.x] += prefix;
}