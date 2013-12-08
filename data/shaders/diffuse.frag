#version 140

in vec3 world_normal; // world-space normal direction
in vec3 view_normal; // view-space normal direction
in vec4 world_pos; // world-space position
in vec3 dir_to_viewer; // normalized vector pointing towards the viewer

out vec4 out_color;

void main()
{
	vec3 light_pos = vec3(5.0, 5.0, 0.0);
	vec3 light_col = vec3(0.95, 0.85, 0.82);
	vec3 ambient = vec3(0.15, 0.18, 0.23);

	// Diffuse
	vec3 dir_to_light = normalize(light_pos - world_pos.xyz);
	float ndotl = dot(dir_to_light, world_normal);
	float incidence = max(ndotl, 0.0);
	vec3 diffuse = incidence * light_col + (1.0 - incidence) * ambient;

	// Specular
	float specular_power = 50.0;
	// vec3 reflected = reflect(world_normal, dir_to_viewer);
	float specular = pow(dot(dir_to_viewer, world_normal), specular_power);

	out_color = vec4(diffuse + specular, 1.0);
}