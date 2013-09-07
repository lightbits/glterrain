#version 140

in vec3 position;
in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 worldNormal;
out vec4 worldPos;
out float distToCamera;
out float height;

void main()
{
	worldPos = model * vec4(position, 1.0);
	vec4 viewPos = view * worldPos;
	gl_Position = projection * viewPos;
	worldNormal = normalize(model * vec4(normal, 0.0));
	distToCamera = length(viewPos);
	height = position.y;
}