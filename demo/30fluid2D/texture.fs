#version 140

in vec2 v_texel;

uniform sampler2D tex;

out vec4 out_color;

void main()
{
	out_color = abs(texture(tex, v_texel));
}