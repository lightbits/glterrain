#version 140

in vec2 vTexel;

uniform sampler2D tex;
uniform vec4 color;

out vec4 outColor;

void main()
{
	outColor = texture(tex, vTexel) * color;
	outColor.rgb *= color.a;
}