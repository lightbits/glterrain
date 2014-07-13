#version 430

float noise1f(int x)
{
	x = (x<<13) ^ x;
    return ( 1.0 - ( (x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

float snoise(float x)
{
	int xi = int(floor(x));
	float xf = x - xi;

	float h0 = noise1f(xi);
	float h1 = noise1f(xi + 1);

	// Smoothly nterpolate between noise values
	float t = smoothstep(0.0, 1.0, xf);
    return h0 + (h1 - h0) * t;
}

layout (local_size_x = 256) in;

layout (std140, binding = 0) buffer DataBuffer {
	vec4 Data[];
};

uniform float time;

void main()
{
	uint index = gl_GlobalInvocationID.x;
	Data[index].x = snoise(time * 0.1 + float(index) * 0.1);
	Data[index].z = snoise(time * 0.1 + float(index) * 0.1 + 100.0);
}