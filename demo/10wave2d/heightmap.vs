#version 140

in vec2 texel;
in vec3 position;

out vec3 v_normal; // View-space normal
out vec3 v_position; // View position

uniform sampler2D heightmap;

uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;
uniform float dx;

void main()
{	
	// Extend y-component based on heightmap
	float h = texture(heightmap, texel).r;
	vec3 p = vec3(position.x, h, position.z);
	v_position = (view * model * vec4(p, 1.0)).xyz;

	// Calculate normal
	float hr = texture(heightmap, texel + vec2(dx, 0.0)).r;
	float hb = texture(heightmap, texel + vec2(0.0, dx)).r;
	vec3 v0 = vec3(dx, hr - h, 0.0);
	vec3 v1 = vec3(0.0, hb - h, dx);
	v_normal = (view * model * vec4(cross(v1, v0), 0.0)).xyz;

	gl_Position = projection * vec4(v_position, 1.0);
}