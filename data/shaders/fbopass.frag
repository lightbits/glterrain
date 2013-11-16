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

	float invbw = 50.0;
	float frequency = 0.3;
	float offset = screenXY.y * 0.3;
	float sharpness = 0.5;

	float scanline = exp(-pow((r - pow(mod(time * 1.1, 2.0), 0.5)) * invbw + 1.0, sharpness));
	// float scanline = exp(-pow((screenXY.y + mod(time * frequency + offset, 1.0) * 2.0 - 1.0) * invbw, sharpness));
	vec4 rValue = texture(tex, vertTexel - vec2(0.01 + sin(time * 1000.0) * 0.1 * scanline, 0.0));
	vec4 gValue = texture(tex, vertTexel - vec2(0.007 + sin(time * 800.0) * 0.02 * scanline, 0.0));
	vec4 bValue = texture(tex, vertTexel - vec2(0.005 + sin(time * 200.0) * 0.2 * scanline, 0.0));
	vec4 color = vec4(rValue.r, gValue.g, bValue.b, 1.0);
	color *= vignette;
	color += scanline * 0.4;
	outColor = color;
}