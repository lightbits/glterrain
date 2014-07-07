#version 430

// Description : Spawn buffer update routine
//      Author : Simen Haugo
//  Maintainer : ARM
//
layout (local_size_x = 128) in;

layout (std140, binding = 0) buffer PositionBuffer {
	vec4 Position[];
};

layout (std430, binding = 1) buffer IndexBuffer {
    uint Index[];
};

layout (std430, binding = 2) buffer KeyBuffer {
    float Key[];
};

uniform vec3 axis;

void main()
{
    uint index = gl_GlobalInvocationID.x;
    Index[index] = index;
    Key[index] = dot(Position[index].xyz, axis);
}