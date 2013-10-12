#version 140

in vec3 position;
in vec3 barycentric;
in vec3 normal;

out vec4 vertColor;
out vec3 vBC;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

void main()
{	
	vertColor = 0.5 * vec4(normal, 1.0) + vec4(0.5);
	// vertColor = vec4(barycentric, 1.0);
	vBC = barycentric; // interpolate barycentric coordinates
	gl_Position = projection * view * model * vec4(position.x, position.y, position.z, 1.0f);
}