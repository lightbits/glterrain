#version 430
in vec3 position;

out vec3 vNormal;
out float depth;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
	vNormal = (view * vec4(position, 0.0)).xyz; // Because the mesh is a sphere!
	gl_Position = projection * view * model * vec4(position, 1.0);
	depth = gl_Position.z * 0.5 + 0.5;
}