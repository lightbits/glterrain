#version 140

in vec3 position;

out vec4 v_color;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

void main()
{	
	gl_Position = projection * view * model * vec4(position, 1.0f);
	v_color = vec4(position * 0.5 + vec3(0.5), 1.0);
}