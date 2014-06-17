#version 430

layout (local_size_x = 16, local_size_y = 16) in;
layout (binding = 0, rgba16f) uniform readonly image2D inTex;
layout (binding = 1, rgba16f) uniform image2D outTex;

void main()
{
	ivec2 texel = ivec2(gl_GlobalInvocationID.xy);

    int kernelSize = 2 + int(float(gl_GlobalInvocationID.x) * 8.0 / imageSize(inTex).x);
    vec4 sum = vec4(0.0);
    for (int y = texel.y - kernelSize; y <= texel.y + kernelSize; ++y)
    {
        for (int x = texel.x - kernelSize; x <= texel.x + kernelSize; ++x)
        {
            sum += imageLoad(inTex, ivec2(x, y));
        }
    }

    imageStore(outTex, texel, sum / float((kernelSize * 2 + 1) * (kernelSize * 2 + 1)));
}