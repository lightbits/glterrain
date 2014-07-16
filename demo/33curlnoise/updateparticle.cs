#version 430

// Description : Curl noise implementation
//      Author : Simen Haugo
//  Maintainer : ARM
layout (local_size_x = 128) in;
layout (binding = 0, rgba8) uniform readonly image2D texNoise;
layout (std140, binding = 0) buffer PositionBuffer {
	vec4 Position[];
};
layout (std140, binding = 1) buffer SpawnBuffer {
	vec4 SpawnInfo[];
};

float hash( float n )
{
    return fract(sin(n) * 43758.5453);
}

// Returns a random value in the range -1.0 -> 1.0
// By Inigo Quilez
float noise( vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);

    f = f * f * (3.0 - 2.0 * f);
    float n = p.x + p.y * 57.0 + 113.0 * p.z;

    //return mix(mix(mix( hash(n+0.0), hash(n+1.0),f.x),
    //               mix( hash(n+57.0), hash(n+58.0),f.x),f.y),
    //           mix(mix( hash(n+113.0), hash(n+114.0),f.x),
    //               mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);

    float a = mix(hash(n + 0.0), hash(n + 1.0), f.x);
    float b = mix(hash(n + 57.0), hash(n + 58.0), f.x);
    float c = mix(hash(n + 113.0), hash(n + 114.0), f.x);
    float d = mix(hash(n + 170.0), hash(n + 171.0), f.x);
    float r = mix(a, b, f.y);
    float s = mix(c, d, f.y);
    return mix(r, s, f.z);
}

vec3 dnoise3f(in vec3 x)
{
    vec3 i = floor(x);
    vec3 u = fract(x);
    vec2 s = vec2(1.0, 0.0);

    vec3 du = 30.0 * u * u * (u * (u - 2.0) + vec3(1.0));
    u = u * u * u * (u * (u * 6.0 - vec3(15.0)) + vec3(10.0));

    float a = noise(i + s.yyy);
    float b = noise(i + s.xyy);
    float c = noise(i + s.yxy);
    float d = noise(i + s.xxy);
    float e = noise(i + s.yyx);
    float f = noise(i + s.xyx);
    float g = noise(i + s.yxx);
    float h = noise(i + s.xxx);

    float k0 =   a;
    float k1 =   b - a;
    float k2 =   c - a;
    float k3 =   e - a;
    float k4 =   a - b - c + d;
    float k5 =   a - c - e + g;
    float k6 =   a - b - e + f;
    float k7 = - a + b + c - d + e - f - g + h;

    vec3 derivatives;
    derivatives.x = du.x * (k1 + k4*u.y + k6*u.z + k7*u.y*u.z); // partial derivative in x direction
    derivatives.y = du.y * (k2 + k5*u.z + k4*u.x + k7*u.z*u.x); // partial derivative in y direction
    derivatives.z = du.z * (k3 + k6*u.x + k5*u.y + k7*u.x*u.y); // partial derivative in z direction
    return derivatives;
}

uniform float dt;
uniform float time;
uniform vec3 seed;
uniform vec3 spherePos;
uniform float particleLifetime;
uniform float sphereRadius;
const float regionLength = 2.0;
const vec2 eps = vec2(0.002, 0.0);

float ramp(float r)
{
    if (r >= 1.0)
        return 1.0;
    else if (r <= -1.0)
        return -1.0;
    float r2 = r * r;
    return r * (1.875 - 1.25 * r2 + 0.375 * r2 * r * r);
}

float phi(vec3 p)
{
    vec3 q = p - spherePos;
    return (3.1415) * log(dot(q, q) + eps.x);
}

void main()
{
	uint index = gl_GlobalInvocationID.x;
    vec4 status = Position[index];
    float lifetime = status.w;
    if (lifetime < 0.0)
    {
        // Respawn particle (note that lifetime is stored in w-component)
        Position[index] = SpawnInfo[index];
    }
    else
    {
        // Update particle
        vec3 p = status.xyz;

        // vec3 scale = vec3(3.0, 3.5, 3.8); // Adjust this to create smaller/greater turbulence
        vec3 scale = vec3(4.0, 4.5, 4.8); // Adjust this to create smaller/greater turbulence

        // Sample the analytic partial derivatives of our three noise functions
        vec3 dN1 = dnoise3f(scale.x * p + seed.xxx + time * 0.1).xyz;
        vec3 dN2 = dnoise3f(scale.y * p + seed.yyy + time * 0.1).xyz;
        vec3 dN3 = dnoise3f(scale.z * p + seed.zzz + time * 0.1).xyz;

	    // Calculate the curl using the partial derivatives
        vec3 v1 = vec3(dN3.y - dN2.z, dN1.z - dN3.x, dN2.x - dN1.y);

        // Calculate the gradient of the potential function
        vec3 v2 = vec3(
            phi(p + eps.xyy) - phi(p - eps.xyy), 
            phi(p + eps.yxy) - phi(p - eps.yxy),
            phi(p + eps.yyx) - phi(p - eps.yyx));
        v2 /= 2.0 * eps.x;

        // We introduce turbulence after the particle has lived for a while
        float a1 = 1.0 - 0.65 * clamp(lifetime / particleLifetime, 0.0, 1.0);
        float a2 = 1.0 - ramp(length(p - spherePos) / regionLength);
        vec3 v = 0.7 * a1 * v1 + 0.2 * a2 * v2;
        p += v * dt;
        Position[index] = vec4(p, status.w - dt);
    }
}