// Description : Curl noise implementation
//      Author : Simen Haugo
//  Maintainer : ARM

#version 430
layout (local_size_x = 16) in;
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

void main()
{
	uint index = gl_GlobalInvocationID.x;
    float lifetime = Status[index].w;
    if (lifetime < 0.0)
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
        vec3 v = vec3(0.0, 0.05, 0.0);
        p += v * dt;
        Position[index] = vec4(p, 1.0);
        Status[index].w = Status[index].w - dt;
    }
}