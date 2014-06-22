#version 430

layout (local_size_x = 4, local_size_y = 4, local_size_z = 4) in;
layout (binding = 0, rgba32f) uniform readonly image3D VelocityIn;
layout (binding = 1, rgba32f) uniform readonly image3D QuantityIn;
layout (binding = 2, rgba32f) uniform writeonly image3D QuantityOut;

uniform float dt;
uniform vec3 textureSize;

void main()
{
	ivec3 texel = ivec3(gl_GlobalInvocationID.xyz);
	vec3 vel = imageLoad(VelocityIn, texel).xyz;
	vel *= textureSize;
	
	// Traverse backwards in time to copy quantity forward
	ivec3 sampleTexel = texel - ivec3(vel * dt);
	vec3 src = imageLoad(QuantityIn, sampleTexel).xyz;
	imageStore(QuantityOut, texel, vec4(src, 0.0));
}