#version 430

layout (local_size_x = 4) in; // Must equal work_group_size in reorder(...) func
layout (std430, binding = 0) buffer ScanBuffer {
	uvec4 Scan[];
};

layout (std430, binding = 2) buffer KeyBuffer {
	uint Key[];
};

layout (std430, binding = 3) buffer FlagBuffer {
	uvec4 Flag[];
};

layout (std430, binding = 4) buffer SortedKeyBuffer {
	uint SortedKey[];
};

void main()
{
    uint global_i = gl_GlobalInvocationID.x;
    uint key = Key[global_i];
    uvec4 scan = Scan[global_i]; // Serves as the offset
    uvec4 flag = Flag[global_i]; // Used to decide which element to reorder

    // Can probably be done with a dot product!
    uint offset = 0;
    if (flag.x == 1)
        offset = scan.x;
    else if (flag.y == 1)
        offset = scan.y;
    else if (flag.z == 1)
        offset = scan.z;
    else if (flag.w == 1)
        offset = scan.w;

    SortedKey[offset] = key;
}