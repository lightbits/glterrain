#version 140

in vec3 view_normal; // view-space normal direction
in vec3 view_position; // view-space position
in vec3 dir_to_viewer; // normalized vector pointing towards the viewer

uniform mat4 view;

out vec4 out_color;

void main()
{
	vec3 Lp = (view * vec4(2.0, 5.0, 0.0, 1.0)).xyz;
	vec3 Ld = vec3(0.95, 0.85, 0.82);
	vec3 ambient = vec3(0.15, 0.18, 0.23);

	vec3 N  = normalize(view_normal);

	// Diffuse
	vec3 L = normalize(Lp - view_position); // direction to light
	float NdotL = max(dot(L, N), 0.0);
	vec3 Id = NdotL * Ld;

	// Specular
	float specular_exp = 70.0;
	vec3 R = reflect(-L, N); // reflect light ray against surface normal
	float NdotV = max(dot(dir_to_viewer, R), 0.0);
	float specular = pow(NdotV, specular_exp);
	vec3 Is = specular * vec3(1.0);

	out_color.rgb = Id + Is;
	out_color.a = 1.0;
}