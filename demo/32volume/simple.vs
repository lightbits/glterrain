#version 140

in vec3 position;

out vec4 vColor;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

void main()
{	
	vec4 viewPos = view * model * vec4(position, 1.0);
	gl_Position = projection * viewPos;

	float ax = abs(position.x);
	float ay = abs(position.y);
	float az = abs(position.z);

	vColor = vec4(0.99, 0.56, 0.59, 1.0);
	// vColor = vec4(position * 0.5 + vec3(0.5), 1.0);
	float depth = abs(viewPos.z);
	vColor *= vec4(1.0 - depth / 4.0);
}