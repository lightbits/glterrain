#version 140

in vec2 position;
in vec2 texel;
in float scale;

uniform sampler2D tex_velocity;

void main()
{
	vec2 u = texture(tex_velocity, texel).rg;
	gl_Position = vec4(position + scale * u * 0.1, 0.0, 1.0);
}