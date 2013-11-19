#version 140

in vec2 vertTexel;

out vec4 outColor;

uniform float time;
uniform sampler2D tex;

void main()
{
	vec2 screenXY = vertTexel * 2.0 - 1.0;
	float r = length(screenXY);
	float vignette = exp(-pow(r, 5) * 0.2);
	vec4 color = texture(tex, vertTexel);
	// color *= vignette;
	outColor = color;
}