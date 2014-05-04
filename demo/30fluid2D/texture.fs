#version 140

in vec2 v_texel;

uniform sampler2D tex;

out vec4 out_color;

void main()
{
	// TODO: Remap
	out_color = vec4(texture(tex, v_texel).r, 0.0, 0.0, 1.0);
}