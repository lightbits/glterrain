#version 430

layout (local_size_x = 1, local_size_y = 1, local_size_z = 32) in;
layout (binding = 0, r8ui) uniform uimage3D texInput;
layout (binding = 1, r8ui) uniform uimage3D texOutput;

shared uint sharedData[gl_WorkGroupSize.z];

void main()
{
    const ivec3 global_i = ivec3(gl_GlobalInvocationID);
    const uint local_i = gl_LocalInvocationID.z;
    const uint steps = uint(log2(gl_WorkGroupSize.x)) + 1;

    sharedData[local_i] = imageLoad(texInput, global_i).r;
    barrier();

    for (uint step = 0; step < steps; step++)
    {
        uint rd_id = (1 << step) * (local_i >> step) - 1;
        uint mask = (local_i & (1 << step)) >> step;
        sharedData[local_i] += sharedData[rd_id] * mask;
        barrier();
    }

    imageStore(texOutput, global_i, uvec4(sharedData[local_i]));
}