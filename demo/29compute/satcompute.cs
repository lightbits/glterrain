#version 430

layout (local_size_x = 256) in;
layout (binding = 0, rgba32f) uniform readonly image2D inTex;
layout (binding = 1, rgba32f) uniform image2D outTex;

shared vec3 sharedData[gl_WorkGroupSize.x];

void main()
{
    const ivec2 texel = ivec2(gl_GlobalInvocationID.x % imageSize(inTex).x, gl_GlobalInvocationID.x / imageSize(inTex).y);
    const uint id = gl_LocalInvocationID.x;
    const uint steps = uint(log2(gl_WorkGroupSize.x)) + 1;

    vec4 i0 = imageLoad(inTex, texel);
    sharedData[id] = i0.rgb;
    barrier();

    for (uint step = 0; step < steps; step++)
    {
        uint rd_id = (1 << step) * (id >> step) - 1;
        uint mask = (id & (1 << step)) >> step;
        sharedData[id] += sharedData[rd_id] * mask;
        barrier();
    }

    imageStore(outTex, texel.yx, vec4(sharedData[id], i0.a));
}