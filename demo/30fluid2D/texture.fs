#version 140

in vec2 v_texel;

uniform sampler2D tex;
uniform float amplification;

out vec4 out_color;

void main()
{
	out_color = amplification * texture(tex, v_texel) * 0.5 + vec4(0.5);
}