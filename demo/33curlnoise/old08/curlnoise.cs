// Description : Curl noise implementation
//      Author : Simen Haugo
//  Maintainer : ARM

#version 430
layout (local_size_x = 32) in;
layout (std140, binding = 0) buffer PositionBuffer {
	vec4 Position[];
};
layout (std140, binding = 1) buffer StatusBuffer {
	vec4 Status[];
};
layout (std140, binding = 2) buffer SpawnBuffer {
	vec4 SpawnInfo[];
};

uniform float dt;
const vec3 sphereCenter = vec3(0.0, 0.0, 0.0);
const float sphereRadius = 1.0;
const float regionLength = 2.0;
const vec2 eps = vec2(0.0001, 0.0);

float ramp(float r)
{
    if (r >= 1.0)
        return 1.0;
    else if (r <= -1.0)
        return -1.0;
    float r2 = r * r;
    return r * (1.875 - 1.25 * r2 + 0.375 * r2 * r * r);
}

float sdSphere(vec3 p, float r)
{
    return length(p) - r;
}

void distanceField(vec3 p, out float distance, out vec3 normal)
{
    distance = length(p - sphereCenter) - sphereRadius;
    normal = normalize(p - sphereCenter);
}

vec3 potential(vec3 p)
{
    const float u0 = 1.5; // free stream velocity
    vec3 psi = vec3(u0 * p.z, 0.0, 0.0); // vertical uniform flow field
    
    float distance;
    vec3 normal;
    distanceField(p, distance, normal);
    
    // Ramp down tangential component near the boundary
    float alpha = abs(ramp(distance / regionLength));
    return alpha * psi + (1.0 - alpha) * dot(normal, psi) * normal;
}

void main()
{
	uint index = gl_GlobalInvocationID.x;
    float lifetime = Status[index].w;
    if (Position[index].y > 4.0)
    {
        // Respawn particle
        vec4 info = SpawnInfo[index]; // x, y, z, lifetime
        Position[index] = vec4(info.xyz, 1.0);
        Status[index].w = info.w;
    }
    else
    {
        // Update particle
        vec3 p = Position[index].xyz;
        
        float D3Dy = potential(p + eps.yxy).z - potential(p - eps.yxy).z;
	    float D2Dz = potential(p + eps.yyx).y - potential(p - eps.yyx).y;
	    float D1Dz = potential(p + eps.yyx).x - potential(p - eps.yyx).x;
	    float D3Dx = potential(p + eps.xyy).z - potential(p - eps.xyy).z;
	    float D2Dx = potential(p + eps.xyy).y - potential(p - eps.xyy).y;
	    float D1Dy = potential(p + eps.yxy).x - potential(p - eps.yxy).x;

        vec3 v = vec3(D3Dy - D2Dz, D1Dz - D3Dx, D2Dx - D1Dy);
        v /= 2.0 * eps.x;
        p += v * dt;
        Position[index] = vec4(p, 1.0);
        Status[index].w = Status[index].w - dt;
    }
}