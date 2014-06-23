#version 430

in vec2 vTexel;

uniform sampler2D tex;
uniform vec2 res;

out vec4 outColor;

void main()
{
	vec4 num = vec4(0.0);
	float div = 0.0;
	float dx = 1.0 / res.x;
	float dy = 1.0 / res.y;
	for (int y = -2; y <= 2; ++y)
	{
		for (int x = -2; x <= 2; ++x)
		{
			vec2 texel = vTexel + vec2(x * dx, y * dy);
			vec4 color = texture(tex, texel);
			num += color;
			div += 1.0;
		}
	}
	outColor = num / div;
}