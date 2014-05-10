#version 140

in vec2 v_texel;

uniform sampler2D tex_source;
uniform vec2 offset;
uniform float scale;

out vec4 result;

void main()
{
	result = scale * texture(tex_source, v_texel + offset);
}