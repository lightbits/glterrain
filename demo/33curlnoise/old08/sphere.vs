#version 430
in vec3 position;

out vec3 vNormal;
out float vDistToCamera;

uniform float sinkSourceBlend;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
	float scale = 1.0 - sinkSourceBlend * 0.5;
	vec4 viewPos = view * model * vec4(scale * position, 1.0);
	gl_Position = projection * viewPos;

	vDistToCamera = length(viewPos);
	vNormal = (view * vec4(position, 0.0)).xyz; // Because the mesh is a sphere!
}