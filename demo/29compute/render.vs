#version 430
in vec3 position;
in vec4 particlePosition; // Instanced attribute

out vec3 vNormal;
out vec3 color;
out float depth;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform mat4 mvp;

const vec3 colors[] = { 
	vec3(1.0, 0.3, 0.3),
	vec3(0.35, 1.0, 0.35),
	vec3(0.35, 0.44, 0.88),
	vec3(0.55, 0.88, 0.58),
	vec3(0.99, 0.92, 0.44)
};

void main()
{
	mat4 translate = mat4(1.0);
	translate[3].xyz = particlePosition.xyz;

	vNormal = (view * vec4(position, 0.0)).xyz; // Because the mesh is a sphere!
	gl_Position = projection * view * translate * model * vec4(position, 1.0);
	depth = gl_Position.z * 0.5 + 0.5;

	color = colors[gl_InstanceID % 5];
}