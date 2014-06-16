#version 430

layout (local_size_x = 4, local_size_y = 4) in;
layout (binding = 0, rgba16f) uniform readonly image2D inTex;
layout (binding = 1, rgba16f) uniform image2D outTex;

shared vec4 sharedData[gl_WorkGroupSize.x];

void main()
{
    const ivec2 texel = ivec2(gl_GlobalInvocationID);
    const uint id = gl_LocalInvocationID.y * gl_WorkGroupSize.x + gl_LocalInvocationID.x;
    const uint steps = uint(log2(gl_WorkGroupSize.x)) + 1;

    sharedData[id] = imageLoad(inTex, texel);
    barrier();

    for (uint step = 0; step < steps; step++)
    {
        uint rd_id = (1 << step) * (id >> step) - 1;
        uint mask = (id & (1 << step)) >> step;
        sharedData[id] += sharedData[rd_id] * mask;
        barrier();
    }

    /* Reference
    vec4 sum = vec4(0.0);
    for (int i = 0; i <= gl_LocalInvocationID.x; i++)
    {
        sum += imageLoad(inTex, ivec2(i, 0));
    }
    */
    
    imageStore(outTex, texel, sharedData[id]);
}