#version 140

in vec2 texel;
in vec2 position;

out vec2 v_texel;

void main()
{	
	v_texel = texel;
	gl_Position = vec4(position, 0.0, 1.0);
}