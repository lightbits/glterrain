#version 140

in vec3 v_view_position;
in vec3 v_view_normal;

uniform vec3 diffuse;

out vec3 out_p;
out vec3 out_n;
out vec4 out_d;

void main()
{
	vec3 N = normalize(v_view_normal);
	out_n.x = N.x;
	out_n.y = N.y;
	out_n.z = sign(N.z);
	
	// out_n = v_view_normal;

	out_p.x = 0.0;
	out_p.y = 0.0;
	out_p.z = v_view_position.z;

	out_d.rgb = diffuse;
	out_d.a = 0.0; // Unused
}