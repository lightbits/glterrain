#version 430

layout (local_size_x = 16) in;
layout (binding = 0, rgba16f) uniform writeonly image2D tex;

layout (std140, binding = 0) buffer PositionBuffer {
	vec4 Position[];
};

layout (std140, binding = 1) buffer VelocityBuffer {
	vec4 Velocity[];
};

const vec3 gravity = vec3(0.0, -9.81, 0.0);
uniform float dt;
// const float dt = 0.005; // timestep
const float eps = 0.0001;

void main()
{
	uint index = gl_GlobalInvocationID.x;

	vec3 p = Position[index].xyz;
	vec3 v = Velocity[index].xyz;

	p += v * dt * 0.5;
	v += gravity * dt * 0.5;

	vec3 n = normalize(p); // normal of central sphere
	if (length(p) < 1.0)
	{
		p = n * (1.0 + eps);
		v = length(v) * n * 0.8;
	}

	if (p.y < -2.0)
	{
		p.y = -2.0 + eps;
		v.y *= -0.8;
	}

	if (p.x < -2.0) { p.x = -2.0 + eps; v.x *= -1.0; }
	if (p.x > +2.0) { p.x = +2.0 - eps; v.x *= -1.0; }
	if (p.z < -2.0) { p.z = -2.0 + eps; v.z *= -1.0; }
	if (p.z > +2.0) { p.z = +2.0 - eps; v.z *= -1.0; }

	Position[index].xyz = p;
	Velocity[index].xyz = v;
}