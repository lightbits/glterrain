#version 140

in vec3 position;
in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 worldNormal;
out vec4 worldPos;

void main()
{
	worldPos = model * vec4(position, 1.0);
	gl_Position = projection * view * worldPos;
	worldNormal = normalize(model * vec4(normal, 0.0));
}