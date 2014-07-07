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

// Description : Spawn buffer update routine
//      Author : Simen Haugo
//  Maintainer : ARM
//
layout (local_size_x = 16) in;

layout (std140, binding = 0) buffer SpawnBuffer {
	vec4 SpawnInfo[];
};

uniform vec3 emitterPos;
uniform float particleLifetime;
uniform float time;

void main()
{
	uint index = gl_GlobalInvocationID.x;
	
	vec3 p;

    // Random offset
	float seed = float(index) * 100.0 * time;
    p.x = snoise(seed);
    p.z = snoise(seed + 13.0);
    p.y = snoise(seed + 127.0);

    // Normalize to get sphere distribution
	p = (0.06 + 0.04 * snoise(seed + 491.0)) * normalize(p);

    // Particle respawns at emitter
    p += emitterPos;

    // New lifetime with slight variation
    float newLifetime = (1.0 + 0.25 * snoise(seed)) * particleLifetime;

    SpawnInfo[index] = vec4(p, newLifetime);
}