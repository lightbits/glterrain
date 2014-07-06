#version 430

in vec3 vColor;

uniform vec3 color;

out vec4 outColor;

void main()
{
	outColor = vec4(color, 1.0) + vec4(vColor, 1.0);	
}