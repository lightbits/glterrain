#version 430
in vec3 position;

out vec3 vPosition;
out vec3 vNormal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
	vNormal = normalize(position);
	vPosition = (model * vec4(position, 1.0)).xyz;
	gl_Position = projection * view * vec4(vPosition, 1.0);
}