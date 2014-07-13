#version 430

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (binding = 0, r8) uniform image3D texOutput;

uniform int mask;

void main()
{
    ivec3 texel = ivec3(gl_GlobalInvocationID);
    ivec3 size = imageSize(texOutput);
    float value = texel.x + texel.y * size.x + texel.z * size.x * size.y;
    value /= float(size.x * size.y * size.z);
    imageStore(texOutput, texel, vec4(value) * mask);
}