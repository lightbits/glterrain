#version 140

in vec3 position;

out vec4 vColor;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

void main()
{	
	gl_Position = projection * view * model * vec4(position, 1.0);
	vColor = vec4(1.0, 0.3, 0.3, 1.0);
	vColor *= min(position.z * 0.5 + 0.8, 1.0);
}