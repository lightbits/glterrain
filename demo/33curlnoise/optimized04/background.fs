#version 430

in vec3 dir;

out vec4 outColor;

void main()
{
	vec3 sun = normalize(vec3(0.0, 0.0, -1.0));
	vec3 rd = normalize(dir);
	float sundot = max(dot(rd, sun), 0.0);

	outColor.rgb = vec3(0.85, 0.95, 1.0) * (0.8 + 0.5 * rd.y);
	outColor.rgb += 0.001 * vec3(1.0, 0.8, 0.4) * pow(sundot, 12.0);
	outColor.a = 1.0;
}