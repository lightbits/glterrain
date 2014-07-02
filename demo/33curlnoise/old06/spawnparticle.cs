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
layout (local_size_x = 64) in;

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
    p.x = 0.1 * snoise((float(index) + time));
    p.z = 0.1 * snoise((float(index) + time) + 13.0);
    p.y = 0.1 * snoise((float(index) + time) + 127.0);
    p = (0.05 + 0.04 * snoise(time)) * normalize(p);
    // p.x += snoise(vec3(index, 0.0, 0.0) * 0.01 + vec3(time) * 10.0);
    // p.y += snoise(vec3(0.0, index, 0.0) * 0.01 + vec3(time) * 10.0);
    // p.z += snoise(vec3(0.0, 0.0, index) * 0.01 + vec3(time) * 10.0);

    // Normalize to get sphere distribution
    //p = (0.1 + 0.08 * snoise(vec3(time) * 0.01)) * normalize(p);

    // Particle respawns at emitter
    p += emitterPos;

    // New lifetime with slight variation
    float newLifetime = (1.0 + 0.25 * snoise(vec3(index))) * particleLifetime;

    SpawnInfo[index] = vec4(p, newLifetime);
}