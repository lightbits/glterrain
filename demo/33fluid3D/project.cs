#version 430

layout (local_size_x = 4, local_size_y = 4, local_size_z = 4) in;
layout (binding = 0, rgba32f) uniform readonly image3D PressureIn;
layout (binding = 1, rgba32f) uniform writeonly image3D VelocityOut;

uniform vec3 cellSize;

void main()
{
	ivec3 texel = ivec3(gl_GlobalInvocationID.xyz);
	vec4 delta = vec4(cellSize, 0.0);
	
	float dC = imageLoad(DivergenceIn, texel).x;
	
	// Compute the gradient of the pressure field
	// by sampling the neighboring cells and taking central differences
	float pL = imageLoad(PressureIn, texel - delta.xww).x;
	float pR = imageLoad(PressureIn, texel + delta.xww).x;
	float pT = imageLoad(PressureIn, texel - delta.wyw).x;
	float pB = imageLoad(PressureIn, texel + delta.wyw).x;
	float pN = imageLoad(PressureIn, texel - delta.wwz).x;
	float pF = imageLoad(PressureIn, texel + delta.wwz).x;
	vec3 gradP = 0.5 * vec3((pR - pL) / cellSize.x,
							(pT - pB) / cellSize.y,
							(pF - pN) / cellSize.z);
							
	// Project the velocity onto its divergence-free component
	// by subtracting the gradient
	vec3 vel = imageLoad(VelocityOut, texel).xyz;
	vel -= gradP;
	imageStore(VelocityOut, texel, vec4(vel, 0.0));
}