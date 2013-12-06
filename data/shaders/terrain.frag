#version 140

in vec4 world_pos; // world-space vertex position
in vec3 world_normal; // world-space normal direction
in vec3 view_normal; // view-space normal direction
in vec3 dir_to_viewer; // normalized vector pointing towards the viewer
in float dist_to_cam;

uniform vec3 light0_pos;
uniform vec3 light0_col;
uniform vec3 ambient;

out vec4 out_color;

void main()
{
	// Diffuse lighting
	vec3 dir_to_light = normalize(light0_pos - world_pos.xyz);
	float ndotl = dot(dir_to_light, world_normal);
	float incidence = max(ndotl, 0.0);
	vec3 diffuse = incidence * light0_col + (1.0 - incidence) * ambient;

	// Gooch lighting
	// https://lva.cg.tuwien.ac.at/ecg/wiki/doku.php?id=students:gooch
	float gooch_factor = 0.5 + ndotl * 0.5;
	vec3 cool = vec3(0.2, 0.2, 0.4);
	vec3 warm = vec3(0.84, 0.6, 0.5);
	float alpha = 0.1;
	float beta = 0.9;
	vec3 cdiff = cool + alpha * diffuse;
	vec3 wdiff = warm + beta * diffuse;
	vec3 gooch = gooch_factor * cdiff + (1.0 - gooch_factor) * wdiff;

	gooch = diffuse;
	out_color = vec4(gooch, 1.0);

  	// Fog
	const float LOG2 = 1.442695;
	const float density = 0.05125;
	vec4 fog_color = vec4(0.77, 0.68, 0.68, 1.0);
	float z = dist_to_cam;
	float fog_alpha = 1.0 - clamp(exp2(-density * density * z * z * LOG2), 0.0, 1.0);
	out_color = mix(out_color, fog_color, fog_alpha);

	float slope = abs(world_normal.y);
	if (slope < 0.8)
		out_color -= vec4(0.07, 0.1, 0.1, 0.0) * 2;
	if (slope > 0.8 && world_pos.y > 0.05)
		out_color += vec4(0.35) * 0.7;
	if (slope > 0.8 && world_pos.y > -0.04 && world_pos.y < 0.02)
		out_color *= vec4(0.5, 0.95, 0.55, 1.0);

	// gamma
	out_color = pow(out_color, vec4(0.45));

	// vignetting
	vec2 q = gl_FragCoord.xy / vec2(640.0, 480.0);
	// q = q * 2.0 - vec2(1.0);
	// out_color *= exp(-pow((q.x * q.x + q.y * q.y), 2.4) * 0.3);
	out_color *= 0.5 + 0.5 * pow(16.0 * q.x * q.y * (1.0 - q.x) * (1.0 - q.y), 0.3);

	// Color based on normal
	// out_color = vec4(world_normal * 0.5 + vec3(0.5), 1.0);
}