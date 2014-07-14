#version 430

in vec4 position;

out vec3 color;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 projectionLight;
uniform mat4 viewLight;
uniform sampler2D shadowMap0;

uniform float debug;

void main()
{
	vec4 viewPos = view * vec4(position.xyz, 1.0);
	float distance = length(viewPos.xyz);
	gl_Position = projection * viewPos;
	gl_PointSize = 16.0 - 6.0 * (distance - 1.0) / (3.0 - 1.0);

	// Project particle position onto lightmap
	vec4 lightPos = projectionLight * viewLight * position;
	vec3 shadowTexel = vec3(0.5) + 0.5 * lightPos.xyz;
	float z = shadowTexel.z;

	vec4 shadow0 = texture(shadowMap0, shadowTexel.xy);
	// vec4 mask0 = vec4(0.0);
	// if (z > 1.00) mask0.w = 1.0;
	// if (z > 0.75) mask0.z = 1.0;
	// if (z > 0.50) mask0.y = 1.0;
	// if (z > 0.25) mask0.x = 1.0;
	vec4 mask0 = clamp( (vec4(z) - vec4(0.00, 0.25, 0.50, 0.75)) * 4.0, vec4(0.0), vec4(1.0));
	shadow0 *= mask0;

	// Sum up contributions
	float shadow = clamp(shadow0.x + shadow0.y + shadow0.z + shadow0.w, 0.0, 1.0);

	// Finally we color the particle
	color = vec3(0.88, 0.78, 0.75) * (1.0 - shadow) + vec3(0.1, 0.12, 0.18) * shadow;

	// Debug
	if (debug > 0.5)
	{
		color *= 0.0001;
		vec3 cw = vec3(1.0, 0.3, 0.3);
		vec3 cz = vec3(0.3, 1.0, 0.3);
		vec3 cy = vec3(0.3, 0.3, 1.0);
		vec3 cx = vec3(1.0, 0.3, 1.0);
		if (z > 0.75) color += cw;
		else if (z > 0.50) color += cz;
		else if (z > 0.25) color += cy;
		else color += cx;
	}
}
