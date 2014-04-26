#version 140

in vec3 v_position;
in vec3 v_normal;

uniform vec3 light_color0;
uniform mat4 light_m0;

uniform vec3 light_color1;
uniform mat4 light_m1;

uniform vec3 diffuse;

out vec4 out_color;

// See [1]. Look at figure 1 and read a few paragraphs of Section 1.2
// [1]: The Irradiance Jacobian for Partially Occluded Polyhedral Sources 
//      (graphics.cornell.edu/pubs/1994/Arv94.pdf)
vec3 arealight(in vec3 Ld, in mat4 M, in vec3 P, in vec3 N)
{
	// Compute corners of arealight in world-space
	// Assuming the arealight's identity transform is flat down the -y axis
	// TODO: Clip polygon if some vertices are below a surface
	vec3 v0 = (M * vec4(-1, 0, +1, 1)).xyz; // top left
	vec3 v1 = (M * vec4(+1, 0, +1, 1)).xyz; // top right
	vec3 v2 = (M * vec4(+1, 0, -1, 1)).xyz; // bottom right
	vec3 v3 = (M * vec4(-1, 0, -1, 1)).xyz; // bottom left

	// Direction vectors from point P to arealight corners
	vec3 L0 = normalize(v0 - P);
	vec3 L1 = normalize(v1 - P);
	vec3 L2 = normalize(v2 - P);
	vec3 L3 = normalize(v3 - P);

	// Make sure we are on the right side of the light
	// by comparing direction to light with light normal
	float tmp = dot(L0, cross(v2 - v0, v1 - v0));
	if (tmp > 0.0) {
		return vec3(0.0);
	}

	// Vector irradiance at point
	// = the sum of weighted dot-cross products of the above vectors
	vec3 I = vec3(0.0);
	I += acos(dot(L0, L1)) * normalize(cross(L0, L1));
	I += acos(dot(L1, L2)) * normalize(cross(L1, L2));
	I += acos(dot(L2, L3)) * normalize(cross(L2, L3));
	I += acos(dot(L3, L0)) * normalize(cross(L3, L0));

	// Irradiance factor at point
	float F = max(dot(I, N), 0.0) / (2.0 * 3.14159265);

	return F * Ld;
}

void main()
{
	vec3 N = normalize(v_normal);

	out_color = vec4(0.0);
	out_color.rgb += arealight(light_color0, light_m0, v_position, N) * diffuse;
	out_color.rgb += arealight(light_color1, light_m1, v_position, N) * diffuse;

	// Gamma
	out_color.rgb = pow(out_color.rgb, vec3(1.0 / 2.2));
}