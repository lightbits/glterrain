#version 140

in vec3 v_normal;
in vec3 v_position;

uniform mat4 view;
uniform samplerCube cubemap;

out vec4 out_color;

void main()
{
	vec3 V = normalize(v_position); // Direction from camera to vertex
	vec3 N = normalize(v_normal);
	vec3 R = normalize(reflect(V, N));
	R = (inverse(view) * vec4(R, 0.0)).xyz;
	out_color = texture(cubemap, R);
	// out_color = vec4(N * 0.5 + vec3(0.5), 1.0);
	// out_color = vec4(0.42, 0.68, 1.0, 1.0);
}