#version 430

in vec3 v_normal;
in vec3 v_position;

uniform mat4 view;
uniform samplerCube cubemap;

out vec4 out_color;

void main()
{
	// Reflection
	vec3 V = normalize(v_position); // Direction from camera to vertex
	vec3 N = normalize(v_normal);
	vec3 R = normalize(reflect(V, N));
	R = (inverse(view) * vec4(R, 0.0)).xyz;
	vec4 reflection = texture(cubemap, R);

	// Refraction
	float fresnel = 1.0 - dot(N, -V); // Schlick approximation

	float ratio = 1.0 / 1.333;
	vec3 T = refract(V, N, ratio);

	// float index = 1.0 / 1.333; // air / water
	// float sint2 = index * length(cross(-V, N));
	// float cost2 = sqrt(1.0 - sint2 * sint2);
	// vec3 X = normalize(cross(N, cross(N, -V)));
	// vec3 T = sint2 * X - cost2 * N;
	T = (inverse(view) * vec4(T, 0.0)).xyz;
	vec4 refraction = texture(cubemap, T);

	out_color = reflection * fresnel + refraction * (1.0 - fresnel);

	//out_color *= 0.0001;

	// out_color += vec4(N * 0.5 + vec3(0.5), 1.0);
	//out_color += vec4(0.42, 0.68, 1.0, 1.0);
}