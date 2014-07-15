#version 430

/*
 * While the scanblock shader computed the prefix sum of the keys
 * in blocks, this shader merges the inidividual blocks into one prefix sum.
*/

layout (local_size_x = 128) in; // Must equal <block_size> in sort.cpp
layout (std430, binding = 0) buffer ScanBuffer {
	uvec4 Scan[];
};

layout (std430, binding = 1) buffer SumsBuffer {
	uvec4 Sums[];
};

layout (std430, binding = 2) buffer InputBuffer {
	vec4 Input[];
};

shared uvec4 sum;
shared uint lastKey;
uniform int bitOffset;
uniform vec3 axis;
uniform float zMin;
uniform float zMax;

/*
 * The particles are sorting by increasing distance along the sorting axis.
 * We find the distance by a simple dot product. But the sorting algorithm
 * needs integer keys (16-bit in this case), so we convert the distance from
 * the range [zMin, zMax] -> [0, 65535].
 * Finally we extract the current working digit (2-bit in this case) from the key.
*/
uint decodeKey(uint index)
{
	float z = dot(Input[index].xyz, axis);
	z = 65535.0 * clamp( (z - zMin) / (zMax - zMin), 0.0, 1.0 );
	return (uint(z) >> bitOffset) & 3;
}

void main()
{
    // This sum value needs to be added to each element in each scan block
    sum = Sums[gl_WorkGroupID.x];

	// Make the value visible to other threads in the group
    memoryBarrierShared();

    // We also need to add the sum from one scan array to the next digit's scan array
    // So we fetch the last scan value from each digit's array
    uint last_i = gl_NumWorkGroups.x * gl_WorkGroupSize.x - 1;
    uvec4 lastScan = Scan[last_i] + Sums[gl_NumWorkGroups.x - 1];

    // We need the inclusive sum, so we also fetch the last key and check if it matches
    // the digit. If it does, add one to that array.
	lastKey = decodeKey(last_i);
    lastScan += uvec4(lastKey == 0 ? 1 : 0, lastKey == 1 ? 1 : 0, lastKey == 2 ? 1 : 0, lastKey == 3 ? 1 : 0);

    // Could we do this more nicely?
    
    // Finally, add up the values to add to the scan arrays
    uvec4 prefix = uvec4(0, lastScan.x, lastScan.x + lastScan.y, lastScan.x + lastScan.y + lastScan.z);

    Scan[gl_GlobalInvocationID.x] += sum + prefix;
}