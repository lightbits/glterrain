#version 430
in vec3 position;

out vec3 vNormal;
out vec3 vPosition;
out float vDistToCamera;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
	vPosition = (model * vec4(position, 1.0)).xyz;
	vec4 viewPos = view * vec4(vPosition, 1.0);
	vDistToCamera = length(viewPos);
	vNormal = (model * vec4(position, 0.0)).xyz;
	gl_Position = projection * viewPos;
}