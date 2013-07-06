#version 140

in vec4 vertColor;
in vec2 vertTexCoord;

out vec4 outColor;

uniform float texBlend;
uniform sampler2D tex;

void main()
{
	outColor = mix(vertColor, texture(tex, vertTexCoord), texBlend);
}