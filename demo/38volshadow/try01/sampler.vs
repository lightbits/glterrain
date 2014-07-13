#version 430

in vec2 position;
out float sampleSum;
uniform sampler3D tex;
uniform int slice;

void main()
{
	gl_PointSize = 2.0;
	gl_Position = vec4(position, 0.0, 1.0);
	sampleSum = 0.0;
	for (int i = 0; i < 32; i++)
	{
		if (i <= slice)
			sampleSum += texture(tex, vec3(position, i / 32.0)).r;
	}
}