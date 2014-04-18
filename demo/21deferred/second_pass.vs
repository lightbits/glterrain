#version 140

in vec2 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

const float z_near = 0.1f;
const float z_far = 20.0f;

void main()
{
	gl_Position = projection * view * model * vec4(position, 0.0, 1.0);

	// For manual projection
	// float z = 2.0f * position.z / (z_near - z_far) + (z_near + z_far) / (z_near - z_far);
	// float w = -position.z;
	// gl_Position *= 0.001;
	// gl_Position += vec4(position.x, position.y, z, w);
}