#version 140

uniform vec3 light_i; // Light intensity
uniform mat4 light_m; // object -> world transformation for arealight
uniform mat4 view;
uniform mat4 inv_projection;

uniform sampler2D tex_p; // Position texture
uniform sampler2D tex_n; // Normal texture
uniform sampler2D tex_d; // Diffuse texture

out vec4 out_color;

/*
P: Vertex position in view-space
N: Vertex normal in view-space
*/
vec3 arealight(in vec3 P, in vec3 N)
{
	// Compute corners of arealight in view-space
	// Assuming the arealight's identity transform is flat down the -y axis
	vec3 v0 = (view * light_m * vec4(-1, 0, +1, 1)).xyz; // top left
	vec3 v1 = (view * light_m * vec4(+1, 0, +1, 1)).xyz; // top right
	vec3 v2 = (view * light_m * vec4(+1, 0, -1, 1)).xyz; // bottom right
	vec3 v3 = (view * light_m * vec4(-1, 0, -1, 1)).xyz; // bottom left

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

	return F * light_i;
}

void main()
{
	vec2 uv;
	uv.x = gl_FragCoord.x / 720.0;
	uv.y = gl_FragCoord.y / 480.0;

	// Reconstruct position from depth
	float z_view = (texture(tex_p, uv)).z;
	float z_near = -0.1;
	float z_far = -20.0;
	float z_ndc = -1 + 2.0 * (z_view - z_near) / (z_far - z_near);
	vec4 P_clip = -z_view * vec4(uv * 2.0 - vec2(1.0), z_ndc, 1.0);
	vec3 P = (inv_projection * P_clip).xyz;
	// vec3 P = (texture(tex_p, uv)).xyz; // Vertex position in view-space
	// if (P.z > -0.001) // Drop background
	//	discard;

	vec2 N_xy = (texture(tex_n, uv)).xy;
	float N_z_sign = (texture(tex_n, uv)).z;
	vec3 N = vec3(N_xy, N_z_sign * sqrt(1.0 - dot(N_xy, N_xy)));
	// vec3 N = (texture(tex_n, uv)).xyz; // Vertex normal in view-space
	// N = normalize(N);

	vec3 diffuse = texture(tex_d, uv).rgb;

	out_color.rgb = arealight(P, N) * diffuse;
	out_color.a = 0.0;

	// out_color.rgb *= 0.0001;
	// out_color.rgb += vec3(uv, 0.5);

	// Gamma
	out_color.rgb = pow(out_color.rgb, vec3(1.0 / 2.2));
}