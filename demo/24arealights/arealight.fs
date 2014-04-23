#version 140

in vec3 v_normal;

out vec4 out_color;

void main()
{
	out_color = vec4(v_normal * 0.5 + vec3(0.5), 1.0);
}