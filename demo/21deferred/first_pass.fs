#version 140

in vec3 v_view_position;
in vec3 v_view_normal;

uniform vec3 diffuse;

out vec3 out_p;
out vec3 out_n;
out vec3 out_d;

void main()
{
	out_p = v_view_position;
	out_n = v_view_normal;
	out_d = diffuse;
}