#version 140

uniform vec4 color;

out vec4 outColor;

void main()
{
	outColor = color;
	// outColor.rgb *= color.a;
}