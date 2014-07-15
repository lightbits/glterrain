#version 430

layout (local_size_x = 128) in; // Must equal <local_size_x> in gen_keys(...)
layout (std430, binding = 0) buffer InputBuffer {
	float Input[];
};

layout (std430, binding = 1) buffer OutputBuffer {
	uint Output[];
};

void main()
{
    uint global_i = gl_GlobalInvocationID.x;
    Output[global_i] = uint(65535.0 * clamp(Input[global_i] / 10.0, 0.0, 1.0));
}