#version 430

in vec4 position;

out vec3 color;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 projectionLight;
uniform mat4 viewLight;
uniform sampler2D shadowMap0;

void main()
{
	vec4 viewPos = view * vec4(position.xyz, 1.0);
	float distance = length(viewPos.xyz);
	gl_Position = projection * viewPos;
	gl_PointSize = 16.0 - 6.0 * (distance - 1.0) / (3.0 - 1.0);

	// Project particle position onto lightmap
	vec4 lightPos = projectionLight * viewLight * position;
	vec3 shadowTexel = vec3(0.5) + 0.5 * lightPos.xyz;

	// This works assuming that the projection matrix has been set up correctly,
	// setting the depth range from zmin to zmax of the particle region
	float z = shadowTexel.z;

	vec4 shadow0 = texture(shadowMap0, shadowTexel.xy);

	// Sum components
	vec4 mask0 = vec4(0.0);
	/*
	mask0.x = clamp((z - 0.00) / 0.25, 0.0, 1.0);
	mask0.y = clamp((z - 0.25) / 0.25, 0.0, 1.0);
	mask0.z = clamp((z - 0.50) / 0.25, 0.0, 1.0);
	mask0.w = clamp((z - 0.75) / 0.25, 0.0, 1.0);
	*/

	// Equivalent form!
	mask0 = clamp( (vec4(z) - vec4(0.00, 0.25, 0.50, 0.75)) * 4.0, vec4(0.0), vec4(1.0));

	shadow0 *= mask0;
	float shadow = shadow0.x + shadow0.y + shadow0.z + shadow0.w;

	// Does not look as pretty...
	/*
	if (z >= 1.00) mask0.w = 1.0;
	if (z >= 0.75) mask0.z = 1.0;
	if (z >= 0.50) mask0.y = 1.0;
	if (z >= 0.25) mask0.x = 1.0;
	shadow0 *= mask0;
	float shadow = shadow0.x + shadow0.y + shadow0.z + shadow0.w;
	shadow = clamp(shadow, 0.0, 1.0);
	*/

	color = vec3(0.88, 0.78, 0.75) * (1.0 - shadow) + vec3(0.1, 0.12, 0.18) * shadow;
	//color += vec3(0.5) + 0.5 * vec3(lpvp.z / lpvp.w);
	// color = vec3(0.5) + 0.5 * position.xyz;
}