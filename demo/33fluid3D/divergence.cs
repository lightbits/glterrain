#version 430

layout (local_size_x = 4, local_size_y = 4, local_size_z = 4) in;
layout (binding = 0, rgba32f) uniform readonly image3D VelocityIn;
layout (binding = 1, rgba32f) uniform writeonly image3D DivergenceOut;

uniform vec3 cellSize;

void main()
{
	ivec3 texel = ivec3(gl_GlobalInvocationID.xyz);
	vec4 delta = vec4(cellSize, 0.0);
	
	// Get velocity values from neighboring cells
	vec3 vL = imageLoad(VelocityIn, texel - delta.xww);
	vec3 vR = imageLoad(VelocityIn, texel + delta.xww);
	vec3 vT = imageLoad(VelocityIn, texel - delta.wyw);
	vec3 vB = imageLoad(VelocityIn, texel + delta.wyw);
	vec3 vN = imageLoad(VelocityIn, texel - delta.wwz);
	vec3 vF = imageLoad(VelocityIn, texel + delta.wwz);
	
	// Compute the divergence using central differences
	float divergence = 0.5 * ((vR.x - vL.x) / delta.x + 
							  (vT.y - vB.y) / delta.y + 
							  (vF.z - vN.z) / delta.z);
	
	imageStore(DivergenceOut, texel, vec4(divergence, 0.0, 0.0, 0.0));
}