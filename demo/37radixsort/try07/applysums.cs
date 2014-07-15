#version 430

layout (local_size_x = 128) in; // Must equal <block_size> in app.cpp
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
shared uvec4 lastScan;
uniform int bitOffset;

void main()
{
    const uint radix = 4;

    // This sum value needs to be added to each element in each scan block
    // It is a vec4 because we keep track of 4 scan arrays (one for each radix digit)
    sum = Sums[gl_WorkGroupID.x];
    barrier();

    // We also need to add the sum from one scan array to the next digit's scan array
    // So we fetch the last scan value from each digit's array 
    // (and also add the last Sums value, because we haven't done that yet)
    uint last_i = gl_NumWorkGroups.x * gl_WorkGroupSize.x - 1;
    lastScan = Scan[last_i] + Sums[gl_NumWorkGroups.x - 1];

    // We need the inclusive sum, so we also fetch the last key and check if it matches
    // the digit. If it does, add one to that array.
    lastKey = (Key[last_i] >> bitOffset) & (radix - 1);
    lastScan += uvec4(lastKey == 0 ? 1 : 0, lastKey == 1 ? 1 : 0, lastKey == 2 ? 1 : 0, lastKey == 3 ? 1 : 0);
    
    // Finally, add up the values to add to the scan arrays
    uvec4 last = uvec4(0, lastScan.x, lastScan.x + lastScan.y, lastScan.x + lastScan.y + lastScan.z);

    Scan[gl_GlobalInvocationID.x] += sum + last;
}