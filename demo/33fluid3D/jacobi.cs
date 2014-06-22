#version 430

layout (local_size_x = 4, local_size_y = 4, local_size_z = 4) in;
layout (binding = 0, rgba32f) uniform readonly image3D DivergenceIn;
layout (binding = 1, rgba32f) uniform readonly image3D PressureIn;
layout (binding = 2, rgba32f) uniform writeonly image3D PressureOut;

uniform vec3 cellSize;

void main()
{
	ivec3 texel = ivec3(gl_GlobalInvocationID.xyz);
	vec4 delta = vec4(cellSize, 0.0);
	
	float dC = imageLoad(DivergenceIn, texel).x;
	
	// Get pressure values from neighboring cells
	float pL = imageLoad(PressureIn, texel - delta.xww).x;
	float pR = imageLoad(PressureIn, texel + delta.xww).x;
	float pT = imageLoad(PressureIn, texel - delta.wyw).x;
	float pB = imageLoad(PressureIn, texel + delta.wyw).x;
	float pN = imageLoad(PressureIn, texel - delta.wwz).x;
	float pF = imageLoad(PressureIn, texel + delta.wwz).x;
	
	// Compute the next iteration for the pressure value
	float p = (pL + pR + pB + pT + pN + pF - dC) / 6.0;
	imageStore(PressureOut, texel, vec4(p, 0.0, 0.0, 0.0));
}