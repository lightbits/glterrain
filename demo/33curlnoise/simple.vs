#version 140

in vec3 position;
in vec4 color;

out vec4 v_color;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

void main()
{	
	vec4 viewPos = view * model * vec4(position, 1.0);
	v_color = color * exp(-length(viewPos) * 0.4 + 2.2);
	gl_Position = projection * viewPos;
}