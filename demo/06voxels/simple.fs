#version 140

in vec3 v_pos;
in vec3 v_normal;

out vec4 out_color;

void main()
{
	vec3 light_pos = vec3(3.0, 4.0, 0.0);
	vec3 light_col = vec3(1.0, 0.95, 0.94);
	vec3 L = normalize(light_pos - v_pos);
	vec3 N = normalize(v_normal);
	float NdotL = max(dot(N, L), 0.0);

	vec3 diffuse = vec3(0.76, 0.76, 0.76);
	out_color.rgb = NdotL * light_col * diffuse + (1.0 - NdotL) * diffuse * 0.2;
	out_color.a = 1.0;
}