#version 140

in vec3 position;
in vec3 normal;

out vec3 vPosition;
out vec3 vNormal;
out float dist;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

void main()
{	
	vPosition = (model * vec4(position, 1.0)).xyz;
	vNormal = (model * vec4(normal, 0.0)).xyz;
	vec4 viewPos = view * vec4(vPosition, 1.0);
	dist = length(viewPos.xyz);
	gl_Position = projection * viewPos;
}