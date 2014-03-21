#version 150

in vec4 vertColor;
in vec2 vertTexCoord;

out vec4 outColor;

uniform float texBlend;
uniform sampler2D tex;

void main()
{
	vec4 multColor = mix(vec4(1, 1, 1, 1), vertColor, texBlend);
	outColor = mix(vertColor, texture(tex, vertTexCoord), texBlend) * multColor;
}