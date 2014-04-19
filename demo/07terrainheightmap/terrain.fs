#version 140

in vec2 v_texel;
in vec3 v_view_pos;
in vec3 v_view_dir;
in vec3 T; // Tangent
in vec3 B; // Bitangent
in vec3 U; // Up

out vec4 out_color;

uniform mat4 model;
uniform mat4 view;
uniform sampler2D tex_normal;
uniform sampler2D tex_diffuse;

const vec3 ks = vec3(1.0, 1.0, 1.0); // Specular reflectance
const vec3 kd = vec3(1.0, 1.0, 1.0); // Diffuse reflectance
const float specular_exp = 100.0;

/*
Lp: Light position in view space
Ld: Light diffuse color
Ls: Light specular color
P: Vertex position in view space
V: Direction to camera
N: Normal in view-space
*/
vec3 phong(in vec3 Lp, in vec3 Ld, in vec3 Ls, in vec3 P, in vec3 V, in vec3 N)
{
	// Diffuse
	vec3 L = normalize(Lp - P);
	float LdotN = max(dot(L, N), 0.0);
	vec3 Id = LdotN * kd * Ld;

	// Specular
	vec3 R = normalize(reflect(-L, N));
	float RdotV = max(dot(R, V), 0.0);
	float specular = pow(RdotV, specular_exp);
	vec3 Is = specular * ks * Ls;

	return (Id + Is);
}

void main()
{
	vec3 albedo = texture(tex_diffuse, v_texel).rgb;

	// Calculate final view-space normal
	vec3 Ns = texture(tex_normal, v_texel).xyz * 2.0 - vec3(1.0);
	vec3 N = Ns.x * T + Ns.y * B + Ns.z * U;
	N = normalize(view * model * vec4(N, 0.0)).xyz;

	// Main light (sun)
	vec3 Lp0 = (view * vec4(0.0, 20.0, 0.0, 1.0)).xyz;
	vec3 Ld0 = vec3(1.0, 0.8, 0.5);
	vec3 Ls0 = vec3(1.0, 1.0, 1.0);

	out_color.rgb = vec3(0.0);
	out_color.rgb += albedo * phong(Lp0, Ld0, Ls0, v_view_pos, v_view_dir, N);
	out_color.a = 1.0;
}