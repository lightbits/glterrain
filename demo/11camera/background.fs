#version 140

in vec3 texel;

uniform samplerCube cubemap;

out vec4 out_color;

void main()
{	
	out_color = texture(cubemap, texel);
	// out_color = vec4(normalize(texel) * 0.5 + vec3(0.5), 1.0);
}