#version 140

in vec2 vertTexel;

uniform sampler2D tex;

out vec4 outColor;

void main()
{
	outColor = texture(tex, vertTexel);
}