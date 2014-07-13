#version 430

in vec2 vTexel;

out vec4 outColor;

uniform float layer;
uniform sampler2DArray tex;

void main()
{
	outColor = texture(tex, vec3(vTexel, layer));
}