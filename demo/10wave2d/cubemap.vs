#version 140

in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 texel;

void main()
{	
	texel = (model * vec4(position, 1.0)).xyz;

	// We want the cube to be centered around the camera,
	// so we disable the translation component of the view
	// matrix by setting w = 0
	vec4 view_pos = view * model * vec4(position, 0.0); 

	// Enable it again for perspective divide
	view_pos.w = 1.0;
	gl_Position = projection * view_pos;
}