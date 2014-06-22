#version 430

uniform sampler3D texVolume;

in vec3 vTexel;

out vec4 outColor;

void main()
{
	float density = texture(texVolume, vTexel).r;
	float fog = 1.0 - exp(-vTexel.z * vTexel.z * 3.2);
	outColor.rgb = (vec3(0.89, 0.82, 0.80)) * fog;
	outColor.a = density;
}