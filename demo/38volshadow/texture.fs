#version 430

in vec2 vTexel;

out vec4 outColor;

uniform sampler2D tex;

void main()
{
	// outColor = texture(tex, vTexel);
	outColor = vec4(texture(tex, vTexel).x);
}