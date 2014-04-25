#version 140

in vec3 v_position;
in vec3 v_normal;

uniform vec3 light_pos0;
uniform vec3 light_color0;
uniform mat4 light_m0;

uniform vec3 light_pos1;
uniform vec3 light_color1;
uniform mat4 light_m1;

uniform vec3 diffuse;

out vec4 out_color;

vec3 arealight(in vec3 Lp, in vec3 Ld, in mat4 M, in vec3 P, in vec3 N)
{
	// Compute dimensions
	vec3 Lu = (M * vec4(0.0, 0.0, 1.0, 0.0)).xyz; // Light height
	vec3 Lr = (M * vec4(1.0, 0.0, 0.0, 0.0)).xyz; // Light width
	vec3 Ln = (M * vec4(0.0, 1.0, 0.0, 0.0)).xyz; // Light normal
	Ln = normalize(Ln);

	// Translate P to the light source's space
	vec3 Pas = P - Lp;

	// Dimensions of arealight
	float w = length(Lr);
	float h = length(Lu);

	// Project onto plane containing arealight
	vec3 u = Lu / h;
	vec3 r = Lr / w;
	float a = dot(Pas, u) / dot(u, u);
	float b = dot(Pas, r) / dot(r, r);
	vec3 P1 = a * u + b * r;

	// Clamp to find closest point inside the arealight
	a = min(max(a, -h / 2.0), +h / 2.0);
	b = min(max(b, -w / 2.0), +w / 2.0);
	vec3 P2 = a * u + b * r;

	// Distance between these points are used to attenuate
	float Dp = length(P2 - P1);

	// Also use the distance from point to center of arealight
	float Dc = length(Lp - P);

	// Attenuation
	vec3 L = normalize(Lp - P);
	float NdotL = 5.0 * max(dot(N, L), 0.0); // How much of the surface is facing the light?
	float LndotL = max(dot(-Ln, L), 0.0); // How much of the light's front is facing the vertex?
	float Id = NdotL * LndotL; // Diffuse intensity

	Id *= 1.0 - min(Dp, 1.0); // Falloff from distance between projected point and clamped point
	Id *= 1.0 / Dc; // Falloff from distance to light source
	
	return Id * Ld;
}

void main()
{
	out_color = vec4(0.0);

	vec3 N = normalize(v_normal);
	

	out_color.rgb += arealight(light_pos0, light_color0, light_m0, v_position, N) * diffuse;
	out_color.rgb += arealight(light_pos1, light_color1, light_m1, v_position, N) * diffuse * 0.001;
	out_color.a = 0.0;

	// Gamma
	out_color.rgb = pow(out_color.rgb, vec3(1.0 / 2.2));
}